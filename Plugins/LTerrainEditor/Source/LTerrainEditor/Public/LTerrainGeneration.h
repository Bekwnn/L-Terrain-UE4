#pragma once
#include "LTerrainEditor.h"
#include "Landscape.h"

class LTerrainGeneration
{
public:
	static void GenerateTerrain(LSystem& lSystem, ALandscape* terrain);

private:
	static ULandscapeLayerInfoObject* CreateLayerInfoAsset(LGroundTexture& layerInfo);
	static float SumNoiseMaps(TArray<LNoisePtr>& noiseMaps, float x, float y);
	static float BilerpEase(float t);
};