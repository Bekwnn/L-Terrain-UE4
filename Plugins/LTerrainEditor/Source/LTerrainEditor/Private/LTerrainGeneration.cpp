#include "LTerrainEditor.h"
#include "LTerrainGeneration.h"

#include "LTerrainComponentMainTask.h"
#include "LFoliageTask.h"

#include "LandscapeComponent.h"
#include "LandscapeEdit.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeLayerInfoObject.h"
#include "Materials/MaterialInstanceConstant.h"

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

void LTerrainGeneration::GenerateTerrain(LSystem& lSystem, ALandscape* terrain)
{
	LSharedTaskParams SP;

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
	FScopedSlowTask loadingBarMain(SP.landscapeComponentCount+1, LOCTEXT("InitializeText", "Initializing Landscape Data"));
	loadingBarMain.MakeDialog();
	loadingBarMain.EnterProgressFrame();

	FOnCompletion onCompletion = FOnCompletion();
	FThreadSafeCounter taskRunningCount(0);
	onCompletion.BindLambda([&taskRunningCount](bool bWasSuccessful) {
		taskRunningCount.Decrement();
	});
	
	//goes positive X for each +1, then positive Y for a row
	for (int compIdx = 0; compIdx < SP.landscapeComponentCount; ++compIdx)
	{
		taskRunningCount.Increment();
		(new FAutoDeleteAsyncTask<FLTerrainComponentMainTask>(compIdx, SP, onCompletion))->StartBackgroundTask();
	}

	//update loading bar while waiting for async tasks to finish
	for (int barCount = taskRunningCount.GetValue(); taskRunningCount.GetValue() > 0 && barCount > 0; )
	{
		while (barCount > taskRunningCount.GetValue())
		{
			loadingBarMain.EnterProgressFrame();
			--barCount;
		}
	}
	///END MAIN LOOP

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
	int foliageTaskCount = 0;
	for (LPatchPtr patch : SP.allUsedPatches)
	{
		for (LObjectScatterPtr objectScatter : patch->objectScatters)
		{
			++foliageTaskCount;
		}
	}

	FScopedSlowTask loadingBarFoliage(foliageTaskCount + 1, LOCTEXT("InitializeText", "Placing Foliage"));
	loadingBarFoliage.MakeDialog();
	loadingBarFoliage.EnterProgressFrame();

	FOnFoliageCompletion onFolaigeCompletion = FOnFoliageCompletion();
	taskRunningCount.Set(0);
	onFolaigeCompletion.BindLambda([&taskRunningCount](bool bWasSuccessful) {
		taskRunningCount.Decrement();
	});

	//goes positive X for each +1, then positive Y for a row
	AInstancedFoliageActor* foliageActor = AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(SP.terrain->GetWorld(), true);
	TArray<LFoliageParams> FPs = TArray<LFoliageParams>();
	for (LPatchPtr patch : SP.allUsedPatches)
	{
		for (LObjectScatterPtr objectScatter : patch->objectScatters)
		{
			int idx = FPs.Add(LFoliageParams());
			FPs[idx].foliageType = Cast<UFoliageType>(objectScatter->meshAsset->foliageType.GetAsset());
			FPs[idx].meshInfo = foliageActor->FindOrAddMesh(FPs[idx].foliageType);
			FPs[idx].patch = patch;
			FPs[idx].objectScatter = objectScatter;
			FPs[idx].RNGSeed = taskRunningCount.GetValue();

			taskRunningCount.Increment();
			(new FAutoDeleteAsyncTask<FLFoliageTask>(FPs[idx], SP, onFolaigeCompletion))->StartBackgroundTask();
		}
	}

	//update loading bar while waiting for async tasks to finish
	for (int barCount = taskRunningCount.GetValue(); taskRunningCount.GetValue() > 0 && barCount > 0; )
	{
		while (barCount > taskRunningCount.GetValue())
		{
			loadingBarFoliage.EnterProgressFrame();
			--barCount;
		}
	}

	//spawn foliage instances from accepted locations
	for (const LFoliageParams& fp : FPs)
	{
		for (const FVector& location : fp.acceptedLocations3D)
		{
			FFoliageInstance instance = FFoliageInstance();
			instance.Location = SP.terrain->GetActorLocation() + location;
			fp.meshInfo->AddInstance(foliageActor, fp.foliageType, instance);
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
