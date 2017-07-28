#pragma once
#include "LTerrainEditor.h"
//Spawns the map editor tab and ui

class SLMapView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLMapView) {}
	SLATE_ARGUMENT(LSymbol2DMapPtr, Map)
	SLATE_END_ARGS()

	void Construct(const FArguments& args);
	void Reconstruct(LSymbol2DMapPtr item);
};