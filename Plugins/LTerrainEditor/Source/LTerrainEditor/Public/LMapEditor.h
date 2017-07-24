#pragma once
#include "LTerrainEditor.h"
#include "SlateCore.h"
#include "Framework/Layout/ScrollyZoomy.h"
#include "Framework/Docking/TabManager.h"

//Spawns the map editor tab and ui

class SLMapEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLMapEditor) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FReply OnAddLoDClicked();
	FReply OnRemoveLoDClicked();

protected:
	TSharedRef<SHorizontalBox> NewBrushBox();
	TSharedRef<SScrollBox> SpamTestScroll();
	TSharedRef<SWidget> BrushMenuTest();

public:
	TSharedPtr<FLTerrainEditorModule> lTerrainModule;
};