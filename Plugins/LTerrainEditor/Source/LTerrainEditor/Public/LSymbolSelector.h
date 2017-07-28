#pragma once
#include "LTerrainEditor.h"
//Spawns the map editor tab and ui

class SLSymbolSelector : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLSymbolSelector) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& args);
	void Reconstruct();

public:
	TSharedPtr<FLTerrainEditorModule> lTerrainModule;
	LSymbolPtr selectedSymbol;
};