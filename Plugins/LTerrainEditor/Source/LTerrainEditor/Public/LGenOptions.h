#pragma once
#include "LTerrainEditor.h"
#include "SlateCore.h"

//Spawns the rule editor tab and ui

class SLGenOptions : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLGenOptions) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& args);

public:
	TSharedPtr<FLTerrainEditorModule> lTerrainModule;
};