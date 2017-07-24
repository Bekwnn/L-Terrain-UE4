#pragma once
#include "LTerrainEditor.h"
#include "SlateCore.h"

//Spawns the rule editor tab and ui

class SLRuleEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLRuleEditor) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FReply OnAddRuleClicked();
	FReply OnRemoveRuleClicked();
	TSharedRef<ITableRow> GenerateListRow(LRulePtr item, const TSharedRef<STableViewBase> &ownerTable);
	void SelectionChanged(LRulePtr item, ESelectInfo::Type selectType);

protected:
	TSharedRef<SHorizontalBox> NewBrushBox();
	TSharedRef<SWidget> BrushMenuTest();

public:
	TSharedPtr<FLTerrainEditorModule> lTerrainModule;

protected:
	TSharedPtr<SListView<LRulePtr>> ruleListWidget;
};