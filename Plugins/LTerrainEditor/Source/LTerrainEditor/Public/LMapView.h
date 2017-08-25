#pragma once
#include "LTerrainEditor.h"
#include "LSymbolSelector.h"
//Spawns the map editor tab and ui

typedef TBaseDelegate<LSymbolPtr> FOnPaint;

class SLMapView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLMapView) {}
	SLATE_ARGUMENT(LSymbol2DMapPtr, Map)
	SLATE_EVENT(FOnPaint, SymbolBrush)
	SLATE_END_ARGS()

	void Construct(const FArguments& args);
	void Reconstruct(LSymbol2DMapPtr item);

protected:
	FOnPaint SymbolBrush;
};