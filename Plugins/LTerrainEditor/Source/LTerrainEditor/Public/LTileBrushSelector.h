#pragma once
#include "LTerrainEditor.h"

class SLTileBrushSelector : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLTileBrushSelector) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

public:
	TSharedPtr<FLTerrainEditorModule> lTerrainModule;
};