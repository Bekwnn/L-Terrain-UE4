#pragma once
#include "LTerrainEditor.h"
#include "Landscape.h"

struct LSharedTaskParams
{
public:
	ALandscape* terrain;
	int32 ComponentSizeVerts;
	int layerCount;
	int landscapeComponentCount;
	int landscapeComponentCountSqrt;
	int sourceSizeX;
	int sourceSizeY;
	TMap<LSymbolPtr, LPatchPtr> symbolPatchMap;
	TArray<LPatchPtr> allUsedPatches;
	FCriticalSection allUsedPatchesLock;
	LSymbol2DMapPtr sourceLSymbolMap;
	TArray<uint16> roughHeightmap;
	TArray<uint16> smoothedHeightMap;
	float metersToU16;
	uint16 zeroHeight;
	LSystem* lSystem;
	TArray<ULandscapeLayerInfoObject*> layerInfos;
	TArray<TArray<FColor>> heightMaps;
	TArray<TArray<TArray<uint8>>> weightMaps;

	//thread safe add unique
	void AddUniqueToUsedPatches(LPatchPtr val)
	{
		allUsedPatchesLock.Lock();
		allUsedPatches.AddUnique(val);
		allUsedPatchesLock.Unlock();
	}
};

class LTerrainGeneration
{
public:
	static void GenerateTerrain(LSystem& lSystem, ALandscape* terrain);

	static ULandscapeLayerInfoObject* CreateLayerInfoAsset(LPaintWeight& layerInfo);
	
	static float SumNoiseMaps(TArray<LNoisePtr>& noiseMaps, float x, float y);
	static float BilerpEase(float t);
	static void GetWeightMapsAt(LSystem& lsystem, TArray<LPaintWeightPtr>& patchPaints, float x, float y, TArray<float>& outWeights, TArray<int>& idxsTouched);
};