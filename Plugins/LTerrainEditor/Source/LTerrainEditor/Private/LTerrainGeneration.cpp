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
				float xPercCoords = (float)(((compIdx % landscapeComponentCountSqrt) * (ComponentSizeVerts - 1)) + j) / (float)(landscapeComponentCountSqrt * (ComponentSizeVerts - 1));
				float yPercCoords = (float)(((compIdx / landscapeComponentCountSqrt) * (ComponentSizeVerts - 1)) + i) / (float)(landscapeComponentCountSqrt * (ComponentSizeVerts - 1));
				float xFloatCoords = xPercCoords * sourceSizeX;
				float yFloatCoords = yPercCoords * sourceSizeY;

				LPatchPtr curPatch = *(symbolPatchMap.Find(LSystem::GetMapSymbolFrom01Coords(sourceLSymbolMap, xPercCoords, yPercCoords)));
				allUsedPatches.AddUnique(curPatch);

				//get 4 indices of source patches surrounding current vert
				int xFloorCoords = FMath::Max(FMath::FloorToInt(xFloatCoords - 0.5f), 0);
				int yFloorCoords = FMath::Max(FMath::FloorToInt(yFloatCoords - 0.5f), 0);
				int xFloorCoordsp1 = FMath::Min(xFloorCoords + 1, sourceSizeX - 1);
				int yFloorCoordsp1 = FMath::Min(yFloorCoords + 1, sourceSizeX - 1);

				//generate noise according to our 4 nearby patches
				float noiseTotalx0y0 = 0.f, noiseTotalx1y0 = 0.f, noiseTotalx0y1 = 0.f, noiseTotalx1y1 = 0.f;
				float noiseX = ((compIdx % landscapeComponentCountSqrt)*(ComponentSizeVerts - 1) + j)*0.1f;
				float noiseY = ((compIdx / landscapeComponentCountSqrt)*(ComponentSizeVerts - 1) + i)*0.1f;
				LPatchPtr patchx0y0 = (*symbolPatchMap.Find((*sourceLSymbolMap)[yFloorCoords][xFloorCoords]));
				LPatchPtr patchx1y0 = (*symbolPatchMap.Find((*sourceLSymbolMap)[yFloorCoords][xFloorCoordsp1]));
				LPatchPtr patchx0y1 = (*symbolPatchMap.Find((*sourceLSymbolMap)[yFloorCoordsp1][xFloorCoords]));
				LPatchPtr patchx1y1 = (*symbolPatchMap.Find((*sourceLSymbolMap)[yFloorCoordsp1][xFloorCoordsp1]));

				//patch x0y0 noise
				TArray<LNoisePtr>& noiseMapList = patchx0y0->noiseMaps;
				noiseTotalx0y0 = SumNoiseMaps(noiseMapList, noiseX, noiseY);

				//patch x1y0 noise
				if (xFloorCoordsp1 == xFloorCoords || patchx1y0 == patchx0y0) { noiseTotalx1y0 = noiseTotalx0y0; }
				else
				{
					noiseMapList = patchx1y0->noiseMaps;
					noiseTotalx1y0 = SumNoiseMaps(noiseMapList, noiseX, noiseY);
				}

				//patch x0y1 noise
				if (yFloorCoordsp1 == yFloorCoords || patchx0y1 == patchx0y0) { noiseTotalx0y1 = noiseTotalx0y0; }
				else
				{
					noiseMapList = patchx0y1->noiseMaps;
					noiseTotalx0y1 = SumNoiseMaps(noiseMapList, noiseX, noiseY);
				}

				//patch x1y1 noise
				if (xFloorCoordsp1 == xFloorCoords || patchx1y0 == patchx0y0) { noiseTotalx1y1 = noiseTotalx0y1; }
				else if (yFloorCoordsp1 == yFloorCoords || patchx0y1 == patchx0y0) { noiseTotalx1y1 = noiseTotalx1y0; }
				else
				{
					noiseMapList = patchx1y1->noiseMaps;
					noiseTotalx1y1 = SumNoiseMaps(noiseMapList, noiseX, noiseY);
				}

				//bilerp fractional coordinates
				//TODO: ease function for bilerpX and bilerpY
				float bilerpX = FMath::Frac(xFloatCoords + 0.5f);
				float bilerpY = FMath::Frac(yFloatCoords + 0.5f);

				//generate large scale height value
				uint16 heightval = (int)FMath::BiLerp(
					(float)roughHeightmap[yFloorCoords   * sourceSizeY + xFloorCoords  ],
					(float)roughHeightmap[yFloorCoords   * sourceSizeY + xFloorCoordsp1],
					(float)roughHeightmap[yFloorCoordsp1 * sourceSizeY + xFloorCoords  ],
					(float)roughHeightmap[yFloorCoordsp1 * sourceSizeY + xFloorCoordsp1],
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

float LTerrainGeneration::SumNoiseMaps(TArray<LNoisePtr>& noiseMaps, float x, float y)
{
	float sum = 0.f;
	for (LNoisePtr noise : noiseMaps)
	{
		sum += noise->Noise(x, y);
	}
	return sum;
}
