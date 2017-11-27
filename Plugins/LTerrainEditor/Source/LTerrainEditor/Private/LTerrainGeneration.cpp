#include "LTerrainEditor.h"
#include "LTerrainGeneration.h"

#include "LTerrainComponentMainTask.h"

#include "LandscapeComponent.h"
#include "LandscapeEdit.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeLayerInfoObject.h"
#include "Materials/MaterialInstanceConstant.h"
#include "InstancedFoliageActor.h"
#include "FoliageType.h"
#include "InstancedFoliage.h"

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

struct Coords
{
public:
	Coords() : x(-1), y(-1) {}
	Coords(int x, int y) : x(x), y(y) {}
	int x;
	int y;
};

void LTerrainGeneration::GenerateTerrain(LSystem& lSystem, ALandscape* terrain)
{
	LSharedTaskParams SP;

	//get the main foliage actor for all foliage instances
	AInstancedFoliageActor* foliageActor = AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(terrain->GetWorld(), true);

	//TArray<FFoliageMeshInfo*> currentFoliages = TArray<FFoliageMeshInfo*>();
	//TArray<UFoliageType*> foliageTypes = TArray<UFoliageType*>();

	SP.terrain = terrain;
	SP.lSystem = &lSystem;

	SP.landscapeComponentCount = terrain->LandscapeComponents.Num();
	SP.landscapeComponentCountSqrt = FMath::Sqrt(SP.landscapeComponentCount);
	if (SP.landscapeComponentCount == 0 || lSystem.lSystemLoDs.Num() == 0) return;

	SP.heightMaps.Init(TArray<FColor>(), SP.landscapeComponentCount);
	SP.weightMaps.Init(TArray<TArray<uint8>>(), SP.landscapeComponentCount);
	SP.patchBlendData.Init(TArray<TArray<float>>(), SP.landscapeComponentCount);

	terrain->Modify();

	//match patches to symbols in the LSystem
	SP.sourceLSymbolMap = lSystem.lSystemLoDs[lSystem.lSystemLoDs.Num() - 1];
	SP.sourceSizeX = (*SP.sourceLSymbolMap).Num();
	SP.sourceSizeY = (*SP.sourceLSymbolMap)[0].Num();
	//ComponentSizeVerts taken from LandscapeEdit.cpp, InitHeightmapData checks size as this squared.
	SP.ComponentSizeVerts = terrain->LandscapeComponents[0]->NumSubsections * (terrain->LandscapeComponents[0]->SubsectionSizeQuads + 1);

	//generate map for symbols and matching patches
	SP.symbolPatchMap = TMap<LSymbolPtr, LPatchPtr>();
	for (LSymbolPtr symbol : lSystem.symbols)
	{
		LPatchPtr matchingPatch = lSystem.GetLPatchMatch(symbol);
		SP.symbolPatchMap.Add(symbol, matchingPatch);
	}

	//after heightmap pass, contains a list of unique patches used in the landscape
	SP.allUsedPatches = TArray<LPatchPtr>();

	//initial rough heightmap
	SP.roughHeightmap = TArray<uint16>();
	SP.roughHeightmap.Reserve(SP.sourceSizeX*SP.sourceSizeY);

	//painting layer data
	SP.layerCount = lSystem.groundTextures.Num();
	SP.layerInfos = TArray<ULandscapeLayerInfoObject*>();
	for (int i = 0; i < SP.layerCount; ++i)
	{
		ULandscapeLayerInfoObject* texLayerInfo = Cast<ULandscapeLayerInfoObject>(lSystem.groundTextures[i]->layerInfo.GetAsset());
		SP.layerInfos.Add(texLayerInfo);
		SP.layerInfos[SP.layerInfos.Num() - 1]->LayerName = FName(*FString::Printf(TEXT("Layer_%d"), i));
	}

	//some constants to be used in generation
	SP.metersToU16 = (UINT16_MAX / 2) / 256.f;
	SP.zeroHeight = UINT16_MAX / 2;

	for (int i = 0; i < SP.sourceSizeY; ++i)
	{
		for (int j = 0; j < SP.sourceSizeX; ++j)
		{
			LPatchPtr curPatch = *SP.symbolPatchMap.Find((*SP.sourceLSymbolMap)[i][j]);
			SP.roughHeightmap.Add(SP.zeroHeight + (int)(FMath::FRandRange(curPatch->minHeight, curPatch->maxHeight) * SP.metersToU16));
		}
	}

	//smooth rough height map before finer detail step
	SP.smoothedHeightMap = SP.roughHeightmap;

	for (int i = 0; i < SP.sourceSizeY; ++i)
	{
		for (int j = 0; j < SP.sourceSizeX; ++j)
		{
			LPatchPtr curPatch = *SP.symbolPatchMap.Find((*SP.sourceLSymbolMap)[i][j]);
			if (!curPatch->bHeightMatch) continue;

			int avgCount = 1;
			float avg = SP.roughHeightmap[i*SP.sourceSizeY + j];
			for (int ii = -1; ii <= 1; ++ii)
			{
				if (i + ii < 0 || i + ii >= SP.sourceSizeY) continue;

				for (int jj = -1; jj <= 1; ++jj)
				{
					if (j + jj < 0 || j + jj >= SP.sourceSizeX) continue;

					avg += (float)SP.roughHeightmap[(i + ii)*SP.sourceSizeY + (j + jj)];
					++avgCount;
				}
			}

			if (avgCount > 1)
			{
				//if we have a useful average, blend towards it
				SP.smoothedHeightMap[i*SP.sourceSizeY + j] = (int)FMath::Lerp(
					(float)SP.smoothedHeightMap[i*SP.sourceSizeY + j],
					avg / avgCount,
					curPatch->heightSmoothFactor);
			}
		}
	}

	///SET UP MAIN LOOP AS ASYNC TASK
	FScopedSlowTask loadingBar(SP.landscapeComponentCount+1, LOCTEXT("InitializeText", "Initializing Landscape Data"));
	loadingBar.MakeDialog();
	loadingBar.EnterProgressFrame();

	FOnCompletion onCompletion = FOnCompletion();
	int taskRunningCount = 0;
	onCompletion.BindLambda([&taskRunningCount](bool bWasSuccessful) {
		--taskRunningCount;
	});
	
	//goes positive X for each +1, then positive Y for a row
	for (int compIdx = 0; compIdx < SP.landscapeComponentCount; ++compIdx)
	{
		++taskRunningCount;
		(new FAutoDeleteAsyncTask<FLTerrainComponentMainTask>(compIdx, SP, onCompletion))->StartBackgroundTask();
	}
	///END MAIN LOOP

	//update loading bar while waiting for async tasks to finish
	for (int barCount = taskRunningCount; taskRunningCount > 0 && barCount > 0; )
	{
		while (barCount > taskRunningCount)
		{
			loadingBar.EnterProgressFrame();
			--barCount;
		}
	}

	///ASSIGN DATA FROM MAIN LOOP (can only be done in main thread)
	for (int compIdx = 0; compIdx < SP.landscapeComponentCount; ++compIdx)
	{
		//APPLY DATA
		ULandscapeComponent* landscapeComponent = terrain->LandscapeComponents[compIdx];
		landscapeComponent->InitHeightmapData(SP.heightMaps[compIdx], false);

		if (SP.layerCount != 0)
			landscapeComponent->InitWeightmapData(SP.layerInfos, SP.weightMaps[compIdx]);


	}
	///END ASSIGNING DATA

	///ASSIGN LANDSCAPE MATERIAL PARAMETERS
	UMaterialInstanceConstant* landscapeMat = Cast<UMaterialInstanceConstant>(terrain->LandscapeMaterial);
	if (landscapeMat != nullptr)
	{
		for (int i = 0; i < lSystem.groundTextures.Num(); ++i)
		{
			if (lSystem.groundTextures[i]->texture.IsValid())
			{
				UTexture2D* diffuse = Cast<UTexture2D>(lSystem.groundTextures[i]->texture.GetAsset());
				if (diffuse != nullptr)
				{
					FName diffuseParamName = FName(*FString::Printf(TEXT("Diffuse_%d"), i));
					landscapeMat->SetTextureParameterValueEditorOnly(diffuseParamName, diffuse);
				}
			}

			if (lSystem.groundTextures[i]->normalMap.IsValid())
			{
				UTexture2D* normalMap = Cast<UTexture2D>(lSystem.groundTextures[i]->normalMap.GetAsset());
				if (normalMap != nullptr)
				{
					FName normMapParamName = FName(*FString::Printf(TEXT("Normal_%d"), i));
					landscapeMat->SetTextureParameterValueEditorOnly(normMapParamName, normalMap);
				}
			}
		}

		
	}
	///ASSIGN LANDSCAPE MATERIAL PARAMETERS END

	///PLACE FOLIAGE AND SCATTER OBJECTS START
	FRandomStream stream = FRandomStream();
	for (LPatchPtr patch : SP.allUsedPatches)
	{
		for (LObjectScatterPtr objectScatter : patch->objectScatters)
		{
			UFoliageType* foliageType = Cast<UFoliageType>(objectScatter->meshAsset->foliageType.GetAsset());
			FFoliageMeshInfo* meshInfo = foliageActor->FindOrAddMesh(foliageType);

			///START Implementation of Fast Poisson Disk Sampling in Arbitrary Dimensions - R. Bridson (2007)
			//pick our initial point near the middle
			TArray<FVector2D> acceptedPointLocations = TArray<FVector2D>();

			//for visual debugging
			//TArray<FVector2D> candidatePointLocations = TArray<FVector2D>();

			//initialize our cell grid
			int uniqueVertCountWidth = SP.landscapeComponentCountSqrt*(SP.ComponentSizeVerts - 1); //width in verts, not counting overlaps at seams
			float realWidthcm = terrain->GetActorScale().X*uniqueVertCountWidth;
			if (objectScatter->minRadius < 0.25f) objectScatter->minRadius = 0.25f;
			float minRadiuscm = objectScatter->minRadius*100.f;
			float maxRadiuscm = objectScatter->maxRadius*100.f;
			float cellSize = minRadiuscm / 1.41421f; //sqrt(n) for n-dimensional
			float cellSizeInv = 1.f / cellSize;
			int gridCount = (realWidthcm / cellSize) + 1; //totalTerrainSize(cm)/cellSize(cm), plus 1 padding
			TArray<TArray<int>> grid = TArray<TArray<int>>(); //2D array of size gridCount x gridCount
			grid.Init(TArray<int>(), gridCount);
			for (int i = 0; i < gridCount; ++i)
			{
				grid[i].Init(-1, gridCount);
			}

			//create and insert the initial point
			//active list points to grid list, grid list points to accepted list
			TArray<Coords> activePoints = TArray<Coords>();
			acceptedPointLocations.Add(FVector2D(
				stream.FRandRange(realWidthcm*0.25f, realWidthcm*0.75f),
				stream.FRandRange(realWidthcm*0.25f, realWidthcm*0.75f)));
			Coords i0gridIdx = Coords((int)(acceptedPointLocations[0].X*cellSizeInv), (int)(acceptedPointLocations[0].Y*cellSizeInv));
			grid[i0gridIdx.x][i0gridIdx.y] = 0; //point to initial point
			activePoints.Add(i0gridIdx);

			while (activePoints.Num() > 0)
			{
				//pick random active point
				int randActivePointIdx = stream.RandRange(0, activePoints.Num() - 1);
				FVector2D activePoint = acceptedPointLocations[grid
					[activePoints[randActivePointIdx].x][activePoints[randActivePointIdx].y]];
				FVector2D candidate;
				float radDist;
				bool candidateFound = false;
				Coords canGridIdx;
				for (int candidateCount = 0; candidateCount < 50; ++candidateCount)
				{
					radDist = stream.FRandRange(minRadiuscm, minRadiuscm*2.f);
					candidate = activePoint + FVector2D(radDist, 0.f).GetRotated(stream.FRandRange(0.f, 359.99f));
					
					// if we're out of bounds, throw candidate out
					if (candidate.X < 0.f || candidate.X >= realWidthcm || candidate.Y < 0.f || candidate.Y >= realWidthcm)
						continue;
					
					canGridIdx = Coords((int)(candidate.X*cellSizeInv), (int)(candidate.Y*cellSizeInv));

					if (grid[canGridIdx.x][canGridIdx.y] >= 0)
					{
						continue; //if this grid spot is occupied, it's too close, throw candidate out
					}
					
					//check neighbor of 8 cells
					candidateFound = true; //mark true until failure found
					for (int i = -2; i <= 2; ++i)
					{
						for (int j = -2; j <= 2; ++j)
						{
							if (i == j && i == 0)
								continue;
							if (canGridIdx.x + i < 0 || canGridIdx.x + i >= gridCount ||
								canGridIdx.y + j < 0 || canGridIdx.y + j >= gridCount)
								continue;

							if (grid[canGridIdx.x + i][canGridIdx.y + j] >= 0) //if neighbor point found
							{
								float neighborDist = FVector2D::Distance(candidate, acceptedPointLocations[grid[canGridIdx.x + i][canGridIdx.y + j]]);
								if (neighborDist < minRadiuscm)
								{
									candidateFound = false;
									goto exitloop;
								}
							}
						}
					}
					if (candidateFound) break;
					exitloop:;
				}

				//valid point, add to accepted list and active point list
				if (candidateFound)
				{
					int addedIdx = acceptedPointLocations.Add(candidate);
					grid[canGridIdx.x][canGridIdx.y] = addedIdx;
					activePoints.Add(canGridIdx);
				}
				else //failed to find new acceptable point, remove from active list
				{
					activePoints.RemoveAt(randActivePointIdx);
				}
			}
			///END Implementation of Fast Poisson Disk Sampling in Arbitrary Dimensions - R. Bridson (2007)

			//trim accepted points based on blend map and obtain Z coordinate
			TArray<FVector> acceptedLocations3D = TArray<FVector>();
			float U16ToMeters = 1.f / SP.metersToU16;
			for (int i = 0; i < acceptedPointLocations.Num(); ++i)
			{
				const FVector2D& point = acceptedPointLocations[i];
				int patchIdx = lSystem.patches.Find(patch);
				//blend data coords as XX.YY, where XX is the landscape component, YY is cordinates within component
				float coordX = (point.X / (realWidthcm / SP.landscapeComponentCountSqrt));
				float coordY = (point.Y / (realWidthcm / SP.landscapeComponentCountSqrt));
				int compIdx = FMath::FloorToInt(coordY) * SP.landscapeComponentCountSqrt + FMath::FloorToInt(coordX); //landscape component
				int subIdx = FMath::FloorToInt(FMath::Frac(coordY) * SP.ComponentSizeVerts) * SP.ComponentSizeVerts + FMath::FloorToInt(FMath::Frac(coordX) * SP.ComponentSizeVerts);//idx inside landscape component
				bool removeInstance = false;
				float blendVal = SP.patchBlendData[compIdx][patchIdx][subIdx];

				//use RNG to toss out some % of instances based on blendVal linearly from 1.0 to 0.5
				if (blendVal < 0.5f) continue;
				else if (blendVal < 0.95f && (stream.GetFraction()/2.f) + 0.5f > blendVal) continue;
				else
				{
					int sIntHeight = ((int)SP.heightMaps[compIdx][subIdx].R << 8) | ((int)SP.heightMaps[compIdx][subIdx].G);
					float terrainZ = (float)(sIntHeight - SP.zeroHeight) * U16ToMeters * 100.f; //to cm
					acceptedLocations3D.Add(FVector(point, terrainZ));
				}
			}

			//spawn foliage instances from accepted locations
			for (const FVector& location : acceptedLocations3D)
			{
				FFoliageInstance instance = FFoliageInstance();
				instance.Location = SP.terrain->GetActorLocation() + location;
				meshInfo->AddInstance(foliageActor, foliageType, instance);
			}
			//for visual debugging
			/*for (const FVector2D& location : candidatePointLocations)
			{
				FFoliageInstance instance = FFoliageInstance();
				instance.Location = SP.terrain->GetActorLocation() + FVector(location, -500.f);
				instance.DrawScale3D = FVector(0.25f, 0.25f, 0.25f);
				meshInfo->AddInstance(foliageActor, foliageType, instance);
			}*/
		}
	}
	///PLACE FOLIAGE AND SCATTER OBJECTS END

	///UPDATE TERRAIN START
	for (ULandscapeComponent* landscapeComponent : terrain->LandscapeComponents)
	{
		landscapeComponent->InvalidateLightingCache();
		landscapeComponent->UpdateCollisionLayerData();
		landscapeComponent->UpdateCachedBounds();
		landscapeComponent->UpdateMaterialInstances();
		//TODO: figure out other functions to call to update lightmap
	}
	///UPDATE TERRAIN END
}
#undef LOCTEXT_NAMESPACE

float LTerrainGeneration::SumNoiseMaps(TArray<LNoisePtr>& noiseMaps, float x, float y)
{
	float sum = 0.f;
	if (noiseMaps.Num() == 0) return sum;

	for (LNoisePtr noise : noiseMaps)
	{
		sum += noise->Noise(x, y);
	}
	return sum;
}

//takes a linear t and returns an ease function t
float LTerrainGeneration::BilerpEase(float t)
{
	//going to use same ease function as perlin noise for now
	return t * t * t * (t * (t * 6 - 15) + 10);
}

//passes idxsTouched to track which textures are used so we can ultimately reduce the amount of paint layers we bilerp
void LTerrainGeneration::GetWeightMapsAt(LSystem& lsystem, TArray<LPaintWeightPtr>& patchPaints, float x, float y, TArray<float>& outWeights, TArray<int>& idxsTouched)
{
	outWeights.Init(0.f, lsystem.groundTextures.Num());

	for (LPaintWeightPtr paintWeight : patchPaints)
	{
		if (!paintWeight->texture.IsValid()) continue;

		int idx = lsystem.groundTextures.Find(paintWeight->texture);
		idxsTouched.AddUnique(idx);

		outWeights[idx] = 1.f;
		/*
		float noiseVal = paintWeight->noiseMap->Noise(x, y);
		float halfFeather = paintWeight->thresholdFeather / 2;
		float alpha = FMath::Clamp((noiseVal - (paintWeight->threshold + halfFeather)) / paintWeight->thresholdFeather, 0.f, 1.f);
		float weight = FMath::Lerp(0.f, 1.f, alpha);

		outWeights[idx] = weight;
		*/
	}
}
