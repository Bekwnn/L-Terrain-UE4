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

	//generate map for symbols and matching patches
	TMap<LSymbolPtr, LPatchPtr> symbolPatchMap = TMap<LSymbolPtr, LPatchPtr>();
	for (LSymbolPtr symbol : lSystem.symbols)
	{
		LPatchPtr matchingPatch = lSystem.GetLPatchMatch(symbol);
		symbolPatchMap.Add(symbol, matchingPatch);
	}

	//some constants to be used in generation
	float u16toMeters = (UINT16_MAX / 2) / 256.f;
	uint16 zeroHeight = UINT16_MAX / 2;
	
	//goes positive X each +1, then positive Y for a row
	for (int compIdx = 0; compIdx < landscapeComponentCount; ++compIdx)
	{
		ULandscapeComponent* landscapeComponent = terrain->LandscapeComponents[compIdx];

		//ComponentSizeVerts taken from LandscapeEdit.cpp, InitHeightmapData checks size as this squared.
		int32 ComponentSizeVerts = landscapeComponent->NumSubsections * (landscapeComponent->SubsectionSizeQuads + 1);
		TArray<FColor> hmapdata = TArray<FColor>();
		hmapdata.Reserve(FMath::Square(ComponentSizeVerts));

		for (int i = 0; i < ComponentSizeVerts; ++i)
		{
			for (int j = 0; j < ComponentSizeVerts; ++j)
			{
				float xPercCoords = (float)(((compIdx % landscapeComponentCountSqrt) * ComponentSizeVerts) + j) / (float)(landscapeComponentCountSqrt * ComponentSizeVerts);
				float yPercCoords = (float)(((compIdx / landscapeComponentCountSqrt) * ComponentSizeVerts) + i) / (float)(landscapeComponentCountSqrt * ComponentSizeVerts);
				LPatchPtr curPatch = *(symbolPatchMap.Find(LSystem::GetMapSymbolFrom01Coords(sourceLSymbolMap, xPercCoords, yPercCoords)));
				//generate height value based on patch settings
				
				//TODO better heightmap
				uint16 heightval = zeroHeight + (int)(FMath::FRandRange(curPatch->minHeight, curPatch->maxHeight) * u16toMeters);

				//data stored in RGBA 32 bit format, RG is 16 bit heightmap data
				hmapdata.Add(FColor(heightval >> 8, heightval & 0xFF, 0));
			}
		}

		landscapeComponent->InitHeightmapData(hmapdata, true);
		landscapeComponent->UpdateMaterialInstances();
		landscapeComponent->UpdateCollisionLayerData();
		landscapeComponent->UpdateCachedBounds();
		//TODO: figure out other functions to call to update lightmap, correct component seams
	}
}

void LTerrainGeneration::GenerateMipMaps(ULandscapeComponent* landscapeComponent)
{
	UTexture2D* tex = landscapeComponent->HeightmapTexture;
	TArray<FColor*> heightMapData;
	for (int i = 0; i < tex->GetNumMips(); ++i)
	{
		heightMapData.Add((FColor*)tex->Source.LockMip(i));
	}
	for (int i = 0; i < tex->GetNumMips()-1; ++i)
	{
		int div = FMath::Pow(2, i + 1);
		int sizeX = tex->GetSizeX() / div;
		int sizeY = tex->GetSizeY() / div;
		for (int X = 0; X < sizeX; ++X)
		{
			for (int Y = 0; Y < sizeY; ++Y)
			{
				int aboveIdx00 = X * 2 * sizeY + Y * 2;
				int aboveIdx01 = aboveIdx00 + 1;
				int aboveIdx10 = aboveIdx00 + 2 * sizeY;
				int aboveIdx11 = aboveIdx10 + 1;

				uint16 average =
					0.25f * (heightMapData[i][aboveIdx00].R << 8 | heightMapData[i][aboveIdx00].G) +
					0.25f * (heightMapData[i][aboveIdx01].R << 8 | heightMapData[i][aboveIdx01].G) +
					0.25f * (heightMapData[i][aboveIdx10].R << 8 | heightMapData[i][aboveIdx10].G) +
					0.25f * (heightMapData[i][aboveIdx11].R << 8 | heightMapData[i][aboveIdx11].G);

				heightMapData[i + 1][X*sizeX + Y].R = average >> 8;
				heightMapData[i + 1][X*sizeX + Y].G = average;
			}
		}
	}
	for (int i = 0; i < tex->GetNumMips(); ++i)
	{
		tex->Source.UnlockMip(i);
	}
}
