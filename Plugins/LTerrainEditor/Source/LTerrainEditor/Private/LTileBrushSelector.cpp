#include "LTerrainEditor.h"
#include "LTileBrushSelector.h"

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

void SLTileBrushSelector::Construct(const FArguments & InArgs)
{
	lTerrainModule = FLTerrainEditorModule::GetModule();
}

#undef LOCTEXT_NAMESPACE