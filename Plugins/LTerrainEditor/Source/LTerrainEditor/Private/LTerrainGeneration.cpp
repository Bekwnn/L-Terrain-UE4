#include "LTerrainEditor.h"
#include "LTerrainGeneration.h"
#include "LandscapeComponent.h"
#include "LandscapeEdit.h"
#include "Landscape.h"
#include "LandscapeInfo.h"

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

	///INITIAL ROUGH HEIGHTMAP
	TArray<uint16> roughHeightmap = TArray<uint16>();
	roughHeightmap.Reserve(sourceSizeX*sourceSizeY);

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

	//TODO: smooth rough height map before finer detail step?
	
	//goes positive X each +1, then positive Y for a row
	for (int compIdx = 0; compIdx < landscapeComponentCount; ++compIdx)
	{
		ULandscapeComponent* landscapeComponent = terrain->LandscapeComponents[compIdx];

		TArray<FColor> hmapdata = TArray<FColor>();
		hmapdata.Reserve(FMath::Square(ComponentSizeVerts));

		for (int i = 0; i < ComponentSizeVerts; ++i)
		{
			for (int j = 0; j < ComponentSizeVerts; ++j)
			{
				float xPercCoords = (float)(((compIdx % landscapeComponentCountSqrt) * ComponentSizeVerts) + j) / (float)(landscapeComponentCountSqrt * ComponentSizeVerts);
				float yPercCoords = (float)(((compIdx / landscapeComponentCountSqrt) * ComponentSizeVerts) + i) / (float)(landscapeComponentCountSqrt * ComponentSizeVerts);
				float xFloatCoords = xPercCoords * sourceSizeX;
				float yFloatCoords = yPercCoords * sourceSizeY;
				int xFloorCoords = FMath::FloorToInt(xFloatCoords-0.5f);
				int yFloorCoords = FMath::FloorToInt(yFloatCoords-0.5f);
				
				LPatchPtr curPatch = *(symbolPatchMap.Find(LSystem::GetMapSymbolFrom01Coords(sourceLSymbolMap, xPercCoords, yPercCoords)));
				allUsedPatches.AddUnique(curPatch);

				//TODO: noise
				//generate height value based on patch settings
				uint16 heightval = (int)FMath::BiLerp(
					(float)roughHeightmap[FMath::Max(yFloorCoords, 0)               * sourceSizeY + FMath::Max(xFloorCoords, 0)              ],
					(float)roughHeightmap[FMath::Max(yFloorCoords, 0)               * sourceSizeY + FMath::Min(xFloorCoords+1, sourceSizeX-1)],
					(float)roughHeightmap[FMath::Min(yFloorCoords+1, sourceSizeY-1) * sourceSizeY + FMath::Max(xFloorCoords, 0)              ],
					(float)roughHeightmap[FMath::Min(yFloorCoords+1, sourceSizeY-1) * sourceSizeY + FMath::Min(xFloorCoords+1, sourceSizeX-1)],
					FMath::Frac(xFloatCoords + 0.5f),
					FMath::Frac(yFloatCoords + 0.5f)
				);

				for (LNoisePtr noise : curPatch->noiseMaps)
				{
					//in a normally scaled landscape a quad is 1x1 meters
					//a frequency of 1.0f repeats every 10 meters if it's a texture
					heightval += metersToU16 * noise->Noise(((compIdx % sourceSizeX) + j)*0.1f, ((compIdx / sourceSizeX) + i)*0.1f);
				}

				//data stored in RGBA 32 bit format, RG is 16 bit heightmap data
				hmapdata.Add(FColor(heightval >> 8, heightval & 0xFF, 0));
			}
		}

		//TODO, parse hmapdata and post process it
		landscapeComponent->InitHeightmapData(hmapdata, true);
	}
	
	int layerCount = 0;
	TArray<ULandscapeLayerInfoObject*> layerInfos = TArray<ULandscapeLayerInfoObject*>();
	TMap<LPatchPtr, int> patchToStartLayer = TMap<LPatchPtr, int>();
	for (LPatchPtr patch : allUsedPatches)
	{
		patchToStartLayer.Add(patch, layerCount);
		for (LGroundTexturePtr tex : patch->groundTextures)
		{
			//TODO: create layerinfo using tex data
			layerInfos.Add(nullptr);
			++layerCount;
		}
	}

	if (layerCount != 0)
	{
		for (int compIdx = 0; compIdx < landscapeComponentCount; ++compIdx)
		{
			ULandscapeComponent* landscapeComponent = terrain->LandscapeComponents[compIdx];

			TArray<TArray<uint8>> weightData = TArray<TArray<uint8>>(); //stored as [layer][datapos]

			//init weight data to 0
			weightData.Init(TArray<uint8>(), layerCount);
			for (int i = 0; i < layerCount; ++i)
			{
				weightData[i].Init(0, FMath::Square(ComponentSizeVerts));
			}

			for (int i = 0; i < ComponentSizeVerts; ++i)
			{
				for (int j = 0; j < ComponentSizeVerts; ++j)
				{
					float xPercCoords = (float)(((compIdx % landscapeComponentCountSqrt) * ComponentSizeVerts) + j) / (float)(landscapeComponentCountSqrt * ComponentSizeVerts);
					float yPercCoords = (float)(((compIdx / landscapeComponentCountSqrt) * ComponentSizeVerts) + i) / (float)(landscapeComponentCountSqrt * ComponentSizeVerts);

					LPatchPtr curPatch = *(symbolPatchMap.Find(LSystem::GetMapSymbolFrom01Coords(sourceLSymbolMap, xPercCoords, yPercCoords)));

					int patchIdxStart = *patchToStartLayer.Find(curPatch);
					for (int texIdx = 0; texIdx < curPatch->groundTextures.Num(); ++texIdx)
					{
						//TODO: generate paint weights (how does ue4 handle weights?)
						weightData[patchIdxStart + texIdx][i*ComponentSizeVerts + j] = 64; //0.25 weight for now
					}
				}
			}

			//landscapeComponent->InitWeightmapData(layerInfos, weightData);
		}
	}

	///UPDATE TERRAIN START
	for (ULandscapeComponent* landscapeComponent : terrain->LandscapeComponents)
	{
		landscapeComponent->UpdateMaterialInstances();
		landscapeComponent->UpdateCollisionLayerData();
		landscapeComponent->UpdateCachedBounds();
		//TODO: figure out other functions to call to update lightmap
	}
	///UPDATE TERRAIN END
}

ULandscapeLayerInfoObject* LTerrainGeneration::CreateLayerInfoAsset(LGroundTexture & layerInfo)
{
	return nullptr;
}
