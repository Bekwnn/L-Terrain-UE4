#include "LTerrainEditor.h"
#include "LTerrainComponentMainTask.h"

#include "LandscapeComponent.h"

FLTerrainComponentMainTask::FLTerrainComponentMainTask(int32 compIdx, LSharedTaskParams& SP, FOnCompletion onCompletion) :
	compIdx(compIdx),
	SP(SP),
	onCompletion(onCompletion)
{
}

void FLTerrainComponentMainTask::DoWork()
{
	ULandscapeComponent* landscapeComponent = SP.terrain->LandscapeComponents[compIdx];

	//init height data
	TArray<FColor> hmapdata = TArray<FColor>();
	hmapdata.Reserve(FMath::Square(SP.ComponentSizeVerts));

	//init weight data to 0
	TArray<TArray<uint8>> weightData = TArray<TArray<uint8>>(); //stored as [layer][datapos]
	weightData.Init(TArray<uint8>(), SP.layerCount);
	for (int i = 0; i < SP.layerCount; ++i)
	{
		weightData[i].Init(0, FMath::Square(SP.ComponentSizeVerts));
	}

	//create foliage instance objects


	///BEGIN MAIN LOOP
	for (int i = 0; i < SP.ComponentSizeVerts; ++i)
	{
		for (int j = 0; j < SP.ComponentSizeVerts; ++j)
		{
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

			///HEIGHT MAP STEPS
			//generate noise according to our 4 nearby patches
			float noiseTotalx0y0 = 0.f, noiseTotalx1y0 = 0.f, noiseTotalx0y1 = 0.f, noiseTotalx1y1 = 0.f;
			float noiseX = ((compIdx % SP.landscapeComponentCountSqrt)*(SP.ComponentSizeVerts - 1) + j)*0.1f;
			float noiseY = ((compIdx / SP.landscapeComponentCountSqrt)*(SP.ComponentSizeVerts - 1) + i)*0.1f;
			LPatchPtr patchx0y0 = (*SP.symbolPatchMap.Find((*SP.sourceLSymbolMap)[yFloorCoords][xFloorCoords]));
			LPatchPtr patchx1y0 = (*SP.symbolPatchMap.Find((*SP.sourceLSymbolMap)[yFloorCoords][xFloorCoordsp1]));
			LPatchPtr patchx0y1 = (*SP.symbolPatchMap.Find((*SP.sourceLSymbolMap)[yFloorCoordsp1][xFloorCoords]));
			LPatchPtr patchx1y1 = (*SP.symbolPatchMap.Find((*SP.sourceLSymbolMap)[yFloorCoordsp1][xFloorCoordsp1]));

			//patch x0y0 noise
			noiseTotalx0y0 = LTerrainGeneration::SumNoiseMaps(patchx0y0->noiseMaps, noiseX, noiseY);

			//patch x1y0 noise
			if (xFloorCoordsp1 == xFloorCoords || patchx1y0 == patchx0y0) { noiseTotalx1y0 = noiseTotalx0y0; }
			else { noiseTotalx1y0 = LTerrainGeneration::SumNoiseMaps(patchx1y0->noiseMaps, noiseX, noiseY); }

			//patch x0y1 noise
			if (yFloorCoordsp1 == yFloorCoords || patchx0y1 == patchx0y0) { noiseTotalx0y1 = noiseTotalx0y0; }
			else { noiseTotalx0y1 = LTerrainGeneration::SumNoiseMaps(patchx0y1->noiseMaps, noiseX, noiseY); }

			//patch x1y1 noise
			if (xFloorCoordsp1 == xFloorCoords || patchx0y1 == patchx1y1) { noiseTotalx1y1 = noiseTotalx0y1; }
			else if (yFloorCoordsp1 == yFloorCoords || patchx1y0 == patchx1y1) { noiseTotalx1y1 = noiseTotalx1y0; }
			else { noiseTotalx1y1 = LTerrainGeneration::SumNoiseMaps(patchx1y1->noiseMaps, noiseX, noiseY); }

			//bilerp fractional coordinates
			float bilerpX = LTerrainGeneration::BilerpEase(FMath::Frac(xFloatCoords + 0.5f));
			float bilerpY = LTerrainGeneration::BilerpEase(FMath::Frac(yFloatCoords + 0.5f));

			//generate large scale height value
			uint16 heightval = (int)FMath::BiLerp(
				(float)SP.smoothedHeightMap[yFloorCoords   * SP.sourceSizeX + xFloorCoords],
				(float)SP.smoothedHeightMap[yFloorCoords   * SP.sourceSizeX + xFloorCoordsp1],
				(float)SP.smoothedHeightMap[yFloorCoordsp1 * SP.sourceSizeX + xFloorCoords],
				(float)SP.smoothedHeightMap[yFloorCoordsp1 * SP.sourceSizeX + xFloorCoordsp1],
				bilerpX,
				bilerpY
			);

			//apply noise
			heightval += (int)(SP.metersToU16 * FMath::BiLerp(
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
			if (SP.layerCount != 0)
			{
				TArray<float> weightsx0y0, weightsx1y0, weightsx0y1, weightsx1y1;
				TArray<int> idxsTouched;

				//patch x0y0 weights
				LTerrainGeneration::GetWeightMapsAt(*SP.lSystem, patchx0y0->paintWeights, noiseX, noiseY, weightsx0y0, idxsTouched);

				//patch x1y0 weights
				if (xFloorCoordsp1 == xFloorCoords || patchx1y0 == patchx0y0) { weightsx1y0 = weightsx0y0; }
				else { LTerrainGeneration::GetWeightMapsAt(*SP.lSystem, patchx1y0->paintWeights, noiseX, noiseY, weightsx1y0, idxsTouched); }

				//patch x0y1 weights
				if (yFloorCoordsp1 == yFloorCoords || patchx0y1 == patchx0y0) { weightsx0y1 = weightsx0y0; }
				else { LTerrainGeneration::GetWeightMapsAt(*SP.lSystem, patchx0y1->paintWeights, noiseX, noiseY, weightsx0y1, idxsTouched); }

				//patch x1y1 weights
				if (xFloorCoordsp1 == xFloorCoords || patchx0y1 == patchx1y1) { weightsx1y1 = weightsx0y1; }
				else if (yFloorCoordsp1 == yFloorCoords || patchx1y0 == patchx1y1) { weightsx1y1 = weightsx1y0; }
				else { LTerrainGeneration::GetWeightMapsAt(*SP.lSystem, patchx1y1->paintWeights, noiseX, noiseY, weightsx1y1, idxsTouched); }

				for (int idx : idxsTouched)
				{
					weightData[idx][i*SP.ComponentSizeVerts + j] = FMath::Clamp<uint8>(
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
	landscapeComponent->InitHeightmapData(hmapdata, false);

	if (SP.layerCount != 0)
		landscapeComponent->InitWeightmapData(SP.layerInfos, weightData);

	FTransform testFoliageT = landscapeComponent->GetComponentToWorld();

	onCompletion.ExecuteIfBound(true); //succeeded in process
}


