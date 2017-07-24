#pragma once
#include "LTerrainEditor.h"
#include "SlateCore.h"

//Spawns the rule editor tab and ui

class SLPatchEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLPatchEditor) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FReply OnAddPatchClicked();
	FReply OnRemovePatchClicked();

protected:
	TSharedRef<SScrollBox> SpamTestScroll();
	TSharedRef<SHorizontalBox> SpawnMatchWidget();

public:
	TSharedPtr<FLTerrainEditorModule> lTerrainModule;
};