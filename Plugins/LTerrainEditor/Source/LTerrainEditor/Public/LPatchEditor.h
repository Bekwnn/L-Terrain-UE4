#pragma once
#include "LTerrainEditor.h"
#include "SlateCore.h"

class SLPatchView;

//Spawns the rule editor tab and ui
class SLPatchEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLPatchEditor) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& args);

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
	TSharedPtr<SLPatchView> patchViewWidget;
};

class SLPatchView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLPatchView) {}
	SLATE_ARGUMENT(LPatchPtr, Patch)
	SLATE_END_ARGS()

	void Construct(const FArguments& args);
	void Reconstruct(LPatchPtr item);
};