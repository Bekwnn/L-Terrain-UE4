#pragma once
#include "LTerrainEditor.h"
//Spawns the map editor tab and ui

typedef TBaseDelegate<void, LSymbolPtr> FOnSelectionClose;

class SLSymbolSelector : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLSymbolSelector) {}
	SLATE_ATTRIBUTE(LSymbolPtr, StartSymbol)
	SLATE_EVENT(FOnSelectionClose, OnSelectionClose)
	SLATE_END_ARGS()

	void Construct(const FArguments& args);
	void Reconstruct();

public:
	TSharedPtr<FLTerrainEditorModule> lTerrainModule;
	LSymbolPtr selectedSymbol;
	FOnSelectionClose _OnSelectionClose;
};