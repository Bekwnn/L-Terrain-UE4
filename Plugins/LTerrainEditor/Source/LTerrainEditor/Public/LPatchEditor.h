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
	TSharedRef<ITableRow> GenerateListRow(LPatchPtr item, const TSharedRef<STableViewBase> &ownerTable);
	void SelectionChanged(LPatchPtr item, ESelectInfo::Type selectType);

protected:
	TSharedRef<SHorizontalBox> SpawnMatchWidget();

public:
	TSharedPtr<FLTerrainEditorModule> lTerrainModule;

protected:
	TSharedPtr<SListView<LPatchPtr>> patchListWidget;
};