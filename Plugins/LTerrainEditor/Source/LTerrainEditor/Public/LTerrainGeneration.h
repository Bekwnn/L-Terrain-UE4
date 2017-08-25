#pragma once
#include "LTerrainEditor.h"
#include "Landscape.h"

class LTerrainGeneration
{
public:
	static void GenerateTerrain(LSystem& lSystem, ALandscape* terrain);

private:
	static void GenerateMipMaps(ULandscapeComponent* landscapeComponent);
};