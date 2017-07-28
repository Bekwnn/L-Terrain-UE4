#pragma once
#include "LTerrainEditor.h"
#include "SlateCore.h"

class SLTileView;

class SLTileEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLTileEditor) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& args);

	FReply OnAddTileClicked();
	FReply OnRemoveTileClicked();
	TSharedRef<ITableRow> GenerateListRow(LSymbolPtr item, const TSharedRef<STableViewBase> &ownerTable);
	void SelectionChanged(LSymbolPtr item, ESelectInfo::Type selectType);

public:
	TSharedPtr<FLTerrainEditorModule> lTerrainModule;

protected:
	TSharedPtr<SListView<LSymbolPtr>> symbolListWidget;
	TSharedPtr<SLTileView> tileViewWidget;
};

class SLTileView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLTileView) {}
	SLATE_ARGUMENT(LSymbolPtr, Symbol)
	SLATE_END_ARGS()

	void Construct(const FArguments& args);
	void Reconstruct(LSymbolPtr item);
};