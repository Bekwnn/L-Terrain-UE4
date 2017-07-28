#pragma once
#include "LTerrainEditor.h"
#include "SlateCore.h"
#include "LSymbolSelector.h"

//Spawns the map editor tab and ui

class SLMapEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLMapEditor) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& args);

	FReply OnAddLoDClicked();
	FReply OnRemoveLoDClicked();
	TSharedRef<ITableRow> GenerateListRow(LSymbol2DMapPtr item, const TSharedRef<STableViewBase> &ownerTable);
	void SelectionChanged(LSymbol2DMapPtr item, ESelectInfo::Type selectType);

protected:
	TSharedRef<SHorizontalBox> NewBrushBox();
	TSharedRef<SScrollBox> SpamTestScroll();
	TSharedRef<SWidget> BrushMenuTest();

public:
	TSharedPtr<FLTerrainEditorModule> lTerrainModule;

protected:
	TSharedPtr<SListView<LSymbol2DMapPtr>> lodListWidget;
	TSharedPtr<SLSymbolSelector> brushWidget;
};