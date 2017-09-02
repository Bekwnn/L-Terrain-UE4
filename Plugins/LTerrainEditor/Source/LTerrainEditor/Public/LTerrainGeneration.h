#pragma once
#include "LTerrainEditor.h"
#include "Landscape.h"

class LTerrainGeneration
{
public:
	static void GenerateTerrain(LSystem& lSystem, ALandscape* terrain);

private:
	static ULandscapeLayerInfoObject* CreateLayerInfoAsset(LPaintWeight& layerInfo);
	static float SumNoiseMaps(TArray<LNoisePtr>& noiseMaps, float x, float y);
	static float BilerpEase(float t);
	static void GetWeightMapsAt(LSystem& lsystem, TArray<LPaintWeightPtr>& patchPaints, float x, float y, TArray<float>& outWeights, TArray<int>& idxsTouched);
};