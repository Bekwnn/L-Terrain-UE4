#pragma once
#include "LTerrainEditor.h"
#include "SlateCore.h"
#include "LSymbolSelector.h"
#include "LMapView.h"

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

public:
	TSharedPtr<FLTerrainEditorModule> lTerrainModule;

protected:
	TSharedPtr<SListView<LSymbol2DMapPtr>> lodListWidget;
	TSharedPtr<SLSymbolSelector> brushWidget;
	TSharedPtr<SLMapView> mapViewWidget;
};