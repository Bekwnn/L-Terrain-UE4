#include "LTerrainEditor.h"
#include "LTerrainComponentMainTask.h"

#include "LandscapeComponent.h"

void FLTerrainComponentMainTask::DoWork()
{
	ULandscapeComponent* landscapeComponent = SP.terrain->LandscapeComponents[compIdx];

	//init height data
	TArray<FColor>& hmapdata = SP.heightMaps[compIdx];
	hmapdata.Reserve(FMath::Square(SP.ComponentSizeVerts));

	//init patch blend data to 0
	TArray<TArray<float>>& patchBlendData = SP.patchBlendData[compIdx];
	patchBlendData.Init(TArray<float>(), SP.lSystem->patches.Num());
	for (int i = 0; i < SP.lSystem->patches.Num(); ++i)
	{
		patchBlendData[i].Init(0.f, FMath::Square(SP.ComponentSizeVerts));
	}

	//init weight data to 0
	TArray<TArray<uint8>>& weightData = SP.weightMaps[compIdx]; //stored as [layer][datapos]
	weightData.Init(TArray<uint8>(), SP.layerCount);
	for (int i = 0; i < SP.layerCount; ++i)
	{
		weightData[i].Init(0, FMath::Square(SP.ComponentSizeVerts));
	}

	///BEGIN MAIN LOOP
	for (int i = 0; i < SP.ComponentSizeVerts; ++i)
	{
		for (int j = 0; j < SP.ComponentSizeVerts; ++j)
		{
			///BUNCH OF GENERAL VARIABLES
			float xPercCoords = (float)(((compIdx % SP.landscapeComponentCountSqrt) * (SP.ComponentSizeVerts - 1)) + j) / (float)(SP.landscapeComponentCountSqrt * (SP.ComponentSizeVerts - 1));
			float yPercCoords = (float)(((compIdx / SP.landscapeComponentCountSqrt) * (SP.ComponentSizeVerts - 1)) + i) / (float)(SP.landscapeComponentCountSqrt * (SP.ComponentSizeVerts - 1));
			float xFloatCoords = xPercCoords * SP.sourceSizeX;
			float yFloatCoords = yPercCoords * SP.sourceSizeY;

			LPatchPtr curPatch = *(SP.symbolPatchMap.Find(LSystem::GetMapSymbolFrom01Coords(SP.sourceLSymbolMap, xPercCoords, yPercCoords)));
			SP.AddUniqueToUsedPatches(curPatch);

			//get 4 indices of source patches surrounding current vert
			int xFloorCoords = FMath::FloorToInt(xFloatCoords - 0.5f);
			int yFloorCoords = FMath::FloorToInt(yFloatCoords - 0.5f);
			int xFloorCoordsp1 = FMath::Min(xFloorCoords + 1, SP.sourceSizeX - 1);
			int yFloorCoordsp1 = FMath::Min(yFloorCoords + 1, SP.sourceSizeY - 1);
			xFloorCoords = FMath::Max(xFloorCoords, 0);
			yFloorCoords = FMath::Max(yFloorCoords, 0);

			float scaledX = ((compIdx % SP.landscapeComponentCountSqrt)*(SP.ComponentSizeVerts - 1) + j)*0.1f;
			float scaledY = ((compIdx / SP.landscapeComponentCountSqrt)*(SP.ComponentSizeVerts - 1) + i)*0.1f;

			//four neighboring patches to vertex
			LPatchPtr patchx0y0 = (*SP.symbolPatchMap.Find((*SP.sourceLSymbolMap)[yFloorCoords][xFloorCoords]));
			LPatchPtr patchx1y0 = (*SP.symbolPatchMap.Find((*SP.sourceLSymbolMap)[yFloorCoords][xFloorCoordsp1]));
			LPatchPtr patchx0y1 = (*SP.symbolPatchMap.Find((*SP.sourceLSymbolMap)[yFloorCoordsp1][xFloorCoords]));
			LPatchPtr patchx1y1 = (*SP.symbolPatchMap.Find((*SP.sourceLSymbolMap)[yFloorCoordsp1][xFloorCoordsp1]));

			TArray<int> patchIdxsTouched = TArray<int>();

			//bilerp fractional coordinates
			float bilerpX = LTerrainGeneration::BilerpEase(FMath::Frac(xFloatCoords + 0.5f));
			float bilerpY = LTerrainGeneration::BilerpEase(FMath::Frac(yFloatCoords + 0.5f));
			///END OF GENERAL VARIABLES

			///CREATE TILE BLEND WEIGHT MAP
			int ix0y0 = 0, ix1y0 = 0, ix0y1 = 0, ix1y1 = 0;
			SP.lSystem->patches.Find(patchx0y0, ix0y0);
			patchBlendData[ix0y0][i*SP.ComponentSizeVerts + j] += (1 - bilerpX)*(1 - bilerpY);
			patchIdxsTouched.AddUnique(ix0y0);

			SP.lSystem->patches.Find(patchx1y0, ix1y0);
			patchBlendData[ix1y0][i*SP.ComponentSizeVerts + j] += (bilerpX)*(1 - bilerpY);
			patchIdxsTouched.AddUnique(ix1y0);

			SP.lSystem->patches.Find(patchx0y1, ix0y1);
			patchBlendData[ix0y1][i*SP.ComponentSizeVerts + j] += (1 - bilerpX)*(bilerpY);
			patchIdxsTouched.AddUnique(ix0y1);

			SP.lSystem->patches.Find(patchx1y1, ix1y1);
			patchBlendData[ix1y1][i*SP.ComponentSizeVerts + j] += (bilerpX)*(bilerpY);
			patchIdxsTouched.AddUnique(ix1y1);
			///END TILE BLEND WEIGHT MAP

			///HEIGHT MAP DATA
			//generate large scale height value
			uint16 heightval = (int)FMath::BiLerp(
				(float)SP.smoothedHeightMap[yFloorCoords   * SP.sourceSizeX + xFloorCoords],
				(float)SP.smoothedHeightMap[yFloorCoords   * SP.sourceSizeX + xFloorCoordsp1],
				(float)SP.smoothedHeightMap[yFloorCoordsp1 * SP.sourceSizeX + xFloorCoords],
				(float)SP.smoothedHeightMap[yFloorCoordsp1 * SP.sourceSizeX + xFloorCoordsp1],
				bilerpX,
				bilerpY
			);

			//noise amount
			float noiseTotal = 0.f;
			for (int patchIdx : patchIdxsTouched)
			{
				noiseTotal +=
					patchBlendData[patchIdx][i*SP.ComponentSizeVerts + j] *
					LTerrainGeneration::SumNoiseMaps(SP.lSystem->patches[patchIdx]->noiseMaps, scaledX, scaledY);
			}
			heightval += (int)(SP.metersToU16 * noiseTotal);

			//data stored in RGBA 32 bit format, RG is 16 bit heightmap data
			hmapdata.Add(FColor(heightval >> 8, heightval & 0xFF, 0));
			///END HEIGHT MAP DATA

			///TEXTURE WEIGHT MAP DATA
			if (SP.layerCount != 0)
			{
				TArray<int> textureIdxsTouched;
				TArray<float> summedWeights = TArray<float>();
				summedWeights.Init(0.f, SP.lSystem->groundTextures.Num());

				for (int patchIdx : patchIdxsTouched)
				{
					TArray<float> weights;
					LTerrainGeneration::GetWeightMapsAt(*SP.lSystem, SP.lSystem->patches[patchIdx]->paintWeights, scaledX, scaledY, weights, textureIdxsTouched);
					for (int texIdx : textureIdxsTouched)
					{
						summedWeights[texIdx] += weights[texIdx] * patchBlendData[patchIdx][i*SP.ComponentSizeVerts + j];
					}
				}

				for (int texIdx : textureIdxsTouched)
				{
					weightData[texIdx][i*SP.ComponentSizeVerts + j] =
						FMath::Clamp<uint8>(summedWeights[texIdx] * 255, 0, 255);
				}
			}
			///END TEXTURE WEIGHT MAP DATA
		}
	}
	onCompletion.ExecuteIfBound(true); //succeeded in process
}


