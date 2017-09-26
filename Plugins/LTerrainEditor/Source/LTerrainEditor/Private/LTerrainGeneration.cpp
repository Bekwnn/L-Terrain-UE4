#include "LTerrainEditor.h"
#include "LTerrainGeneration.h"
#include "LandscapeComponent.h"
#include "LandscapeEdit.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeLayerInfoObject.h"
#include "Materials/MaterialInstanceConstant.h"

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

void LTerrainGeneration::GenerateTerrain(LSystem & lSystem, ALandscape* terrain)
{
	int landscapeComponentCount = terrain->LandscapeComponents.Num();
	int landscapeComponentCountSqrt = FMath::Sqrt(landscapeComponentCount);
	if (landscapeComponentCount == 0 || lSystem.lSystemLoDs.Num() == 0) return;

	terrain->Modify();

	//match patches to symbols in the LSystem
	LSymbol2DMapPtr sourceLSymbolMap = lSystem.lSystemLoDs[lSystem.lSystemLoDs.Num() - 1];
	int sourceSizeX = (*sourceLSymbolMap).Num();
	int sourceSizeY = (*sourceLSymbolMap)[0].Num();
	//ComponentSizeVerts taken from LandscapeEdit.cpp, InitHeightmapData checks size as this squared.
	int32 ComponentSizeVerts = terrain->LandscapeComponents[0]->NumSubsections * (terrain->LandscapeComponents[0]->SubsectionSizeQuads + 1);

	//generate map for symbols and matching patches
	TMap<LSymbolPtr, LPatchPtr> symbolPatchMap = TMap<LSymbolPtr, LPatchPtr>();
	for (LSymbolPtr symbol : lSystem.symbols)
	{
		LPatchPtr matchingPatch = lSystem.GetLPatchMatch(symbol);
		symbolPatchMap.Add(symbol, matchingPatch);
	}

	//after heightmap pass, contains a list of unique patches used in the landscape
	TArray<LPatchPtr> allUsedPatches = TArray<LPatchPtr>();

	//initial rough heightmap
	TArray<uint16> roughHeightmap = TArray<uint16>();
	roughHeightmap.Reserve(sourceSizeX*sourceSizeY);

	//painting layer data
	int layerCount = lSystem.groundTextures.Num();
	TArray<ULandscapeLayerInfoObject*> layerInfos = TArray<ULandscapeLayerInfoObject*>();
	for (int i = 0; i < lSystem.groundTextures.Num(); ++i)
	{
		ULandscapeLayerInfoObject* texLayerInfo = Cast<ULandscapeLayerInfoObject>(lSystem.groundTextures[i]->layerInfo.GetAsset());
		layerInfos.Add(texLayerInfo);
		layerInfos[layerInfos.Num() - 1]->LayerName = FName(*FString::Printf(TEXT("Layer_%d"), i));
	}

	//some constants to be used in generation
	float metersToU16 = (UINT16_MAX / 2) / 256.f;
	uint16 zeroHeight = UINT16_MAX / 2;

	for (int i = 0; i < sourceSizeY; ++i)
	{
		for (int j = 0; j < sourceSizeX; ++j)
		{
			LPatchPtr curPatch = *symbolPatchMap.Find((*sourceLSymbolMap)[i][j]);
			roughHeightmap.Add(zeroHeight + (int)(FMath::FRandRange(curPatch->minHeight, curPatch->maxHeight) * metersToU16));
		}
	}

	//smooth rough height map before finer detail step
	TArray<uint16> smoothedHeightMap = roughHeightmap;

	for (int i = 0; i < sourceSizeY; ++i)
	{
		for (int j = 0; j < sourceSizeX; ++j)
		{
			LPatchPtr curPatch = *symbolPatchMap.Find((*sourceLSymbolMap)[i][j]);
			if (!curPatch->bHeightMatch) continue;

			int avgCount = 1;
			float avg = roughHeightmap[i*sourceSizeY + j];
			for (int ii = -1; ii <= 1; ++ii)
			{
				if (i + ii < 0 || i + ii >= sourceSizeY) continue;

				for (int jj = -1; jj <= 1; ++jj)
				{
					if (j + jj < 0 || j + jj >= sourceSizeX) continue;

					avg += (float)roughHeightmap[(i + ii)*sourceSizeY + (j + jj)];
					++avgCount;
				}
			}

			if (avgCount > 1)
			{
				//if we have a useful average, blend towards it
				smoothedHeightMap[i*sourceSizeY + j] = (int)FMath::Lerp(
					(float)smoothedHeightMap[i*sourceSizeY + j],
					avg / avgCount,
					curPatch->heightSmoothFactor);
			}
		}
	}

	FScopedSlowTask loadingBar(landscapeComponentCount+1, LOCTEXT("InitializeText", "Initializing Landscape Data"));
	loadingBar.MakeDialog();
	loadingBar.EnterProgressFrame();
	
	//goes positive X each +1, then positive Y for a row
	for (int compIdx = 0; compIdx < landscapeComponentCount; ++compIdx)
	{
		loadingBar.EnterProgressFrame();

		ULandscapeComponent* landscapeComponent = terrain->LandscapeComponents[compIdx];

		//init height data
		TArray<FColor> hmapdata = TArray<FColor>();
		hmapdata.Reserve(FMath::Square(ComponentSizeVerts));

		//init weight data to 0
		TArray<TArray<uint8>> weightData = TArray<TArray<uint8>>(); //stored as [layer][datapos]
		weightData.Init(TArray<uint8>(), layerCount);
		for (int i = 0; i < layerCount; ++i)
		{
			weightData[i].Init(0, FMath::Square(ComponentSizeVerts));
		}

		///BEGIN MAIN LOOP
		for (int i = 0; i < ComponentSizeVerts; ++i)
		{
			for (int j = 0; j < ComponentSizeVerts; ++j)
			{
				float xPercCoords = (float)(((compIdx % landscapeComponentCountSqrt) * (ComponentSizeVerts - 1)) + j) / (float)(landscapeComponentCountSqrt * (ComponentSizeVerts - 1));
				float yPercCoords = (float)(((compIdx / landscapeComponentCountSqrt) * (ComponentSizeVerts - 1)) + i) / (float)(landscapeComponentCountSqrt * (ComponentSizeVerts - 1));
				float xFloatCoords = xPercCoords * sourceSizeX;
				float yFloatCoords = yPercCoords * sourceSizeY;

				LPatchPtr curPatch = *(symbolPatchMap.Find(LSystem::GetMapSymbolFrom01Coords(sourceLSymbolMap, xPercCoords, yPercCoords)));
				allUsedPatches.AddUnique(curPatch);

				//get 4 indices of source patches surrounding current vert
				int xFloorCoords = FMath::FloorToInt(xFloatCoords - 0.5f);
				int yFloorCoords = FMath::FloorToInt(yFloatCoords - 0.5f);
				int xFloorCoordsp1 = FMath::Min(xFloorCoords + 1, sourceSizeX-1);
				int yFloorCoordsp1 = FMath::Min(yFloorCoords + 1, sourceSizeY-1);
				xFloorCoords = FMath::Max(xFloorCoords, 0);
				yFloorCoords = FMath::Max(yFloorCoords, 0);

				///HEIGHT MAP STEPS
				//generate noise according to our 4 nearby patches
				float noiseTotalx0y0 = 0.f, noiseTotalx1y0 = 0.f, noiseTotalx0y1 = 0.f, noiseTotalx1y1 = 0.f;
				float noiseX = ((compIdx % landscapeComponentCountSqrt)*(ComponentSizeVerts - 1) + j)*0.1f;
				float noiseY = ((compIdx / landscapeComponentCountSqrt)*(ComponentSizeVerts - 1) + i)*0.1f;
				LPatchPtr patchx0y0 = (*symbolPatchMap.Find((*sourceLSymbolMap)[yFloorCoords][xFloorCoords]));
				LPatchPtr patchx1y0 = (*symbolPatchMap.Find((*sourceLSymbolMap)[yFloorCoords][xFloorCoordsp1]));
				LPatchPtr patchx0y1 = (*symbolPatchMap.Find((*sourceLSymbolMap)[yFloorCoordsp1][xFloorCoords]));
				LPatchPtr patchx1y1 = (*symbolPatchMap.Find((*sourceLSymbolMap)[yFloorCoordsp1][xFloorCoordsp1]));

				//patch x0y0 noise
				noiseTotalx0y0 = SumNoiseMaps(patchx0y0->noiseMaps, noiseX, noiseY);
				
				//patch x1y0 noise
				if (xFloorCoordsp1 == xFloorCoords || patchx1y0 == patchx0y0) { noiseTotalx1y0 = noiseTotalx0y0; }
				else { noiseTotalx1y0 = SumNoiseMaps(patchx1y0->noiseMaps, noiseX, noiseY); }

				//patch x0y1 noise
				if (yFloorCoordsp1 == yFloorCoords || patchx0y1 == patchx0y0) { noiseTotalx0y1 = noiseTotalx0y0; }
				else { noiseTotalx0y1 = SumNoiseMaps(patchx0y1->noiseMaps, noiseX, noiseY); }

				//patch x1y1 noise
				if (xFloorCoordsp1 == xFloorCoords || patchx0y1 == patchx1y1) { noiseTotalx1y1 = noiseTotalx0y1; }
				else if (yFloorCoordsp1 == yFloorCoords || patchx1y0 == patchx1y1) { noiseTotalx1y1 = noiseTotalx1y0; }
				else { noiseTotalx1y1 = SumNoiseMaps(patchx1y1->noiseMaps, noiseX, noiseY); }

				//bilerp fractional coordinates
				float bilerpX = BilerpEase(FMath::Frac(xFloatCoords+0.5f));
				float bilerpY = BilerpEase(FMath::Frac(yFloatCoords+0.5f));

				//generate large scale height value
				uint16 heightval = (int)FMath::BiLerp(
					(float)smoothedHeightMap[yFloorCoords   * sourceSizeX + xFloorCoords  ],
					(float)smoothedHeightMap[yFloorCoords   * sourceSizeX + xFloorCoordsp1],
					(float)smoothedHeightMap[yFloorCoordsp1 * sourceSizeX + xFloorCoords  ],
					(float)smoothedHeightMap[yFloorCoordsp1 * sourceSizeX + xFloorCoordsp1],
					bilerpX,
					bilerpY
				);

				//apply noise
				heightval += (int)(metersToU16 * FMath::BiLerp(
					noiseTotalx0y0,
					noiseTotalx1y0,
					noiseTotalx0y1,
					noiseTotalx1y1,
					bilerpX,
					bilerpY
				));

				//data stored in RGBA 32 bit format, RG is 16 bit heightmap data
				hmapdata.Add(FColor(heightval >> 8, heightval & 0xFF, 0));

				///PAINT MAP STEPS
				if (layerCount != 0)
				{
					TArray<float> weightsx0y0, weightsx1y0, weightsx0y1, weightsx1y1;
					TArray<int> idxsTouched;

					//patch x0y0 weights
					GetWeightMapsAt(lSystem, patchx0y0->paintWeights, noiseX, noiseY, weightsx0y0, idxsTouched);

					//patch x1y0 weights
					if (xFloorCoordsp1 == xFloorCoords || patchx1y0 == patchx0y0) { weightsx1y0 = weightsx0y0; }
					else { GetWeightMapsAt(lSystem, patchx1y0->paintWeights, noiseX, noiseY, weightsx1y0, idxsTouched); }

					//patch x0y1 weights
					if (yFloorCoordsp1 == yFloorCoords || patchx0y1 == patchx0y0) { weightsx0y1 = weightsx0y0; }
					else { GetWeightMapsAt(lSystem, patchx0y1->paintWeights, noiseX, noiseY, weightsx0y1, idxsTouched); }

					//patch x1y1 weights
					if (xFloorCoordsp1 == xFloorCoords || patchx0y1 == patchx1y1) { weightsx1y1 = weightsx0y1; }
					else if (yFloorCoordsp1 == yFloorCoords || patchx1y0 == patchx1y1) { weightsx1y1 = weightsx1y0; }
					else { GetWeightMapsAt(lSystem, patchx1y1->paintWeights, noiseX, noiseY, weightsx1y1, idxsTouched); }

					for (int idx : idxsTouched)
					{
						weightData[idx][i*ComponentSizeVerts + j] = FMath::Clamp<uint8>(
							FMath::BiLerp(
								weightsx0y0[idx], weightsx1y0[idx],
								weightsx0y1[idx], weightsx1y1[idx],
								bilerpX,
								bilerpY
							) * 255,
							0,
							255);
					}
				}
			}
		}

		//APPLY DATA
		landscapeComponent->InitHeightmapData(hmapdata, true);

		if (layerCount != 0)
			landscapeComponent->InitWeightmapData(layerInfos, weightData);
	}
	///END MAIN LOOP

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

	///UPDATE TERRAIN START
	for (ULandscapeComponent* landscapeComponent : terrain->LandscapeComponents)
	{
		landscapeComponent->UpdateCollisionLayerData();
		landscapeComponent->UpdateCachedBounds();
		landscapeComponent->UpdateMaterialInstances();
		//TODO: figure out other functions to call to update lightmap
	}
	///UPDATE TERRAIN END
}
#undef LOCTEXT_NAMESPACE

ULandscapeLayerInfoObject* LTerrainGeneration::CreateLayerInfoAsset(LPaintWeight & layerInfo)
{
	return nullptr;
}

float LTerrainGeneration::SumNoiseMaps(TArray<LNoisePtr>& noiseMaps, float x, float y)
{
	float sum = 0.f;
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
