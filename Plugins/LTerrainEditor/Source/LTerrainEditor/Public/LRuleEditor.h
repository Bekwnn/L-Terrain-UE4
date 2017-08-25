#pragma once
#include "LTerrainEditor.h"
#include "SlateCore.h"
#include "LSymbolSelector.h"

class SLRuleView;

//Spawns the rule editor tab and ui
class SLRuleEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLRuleEditor) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& args);

	FReply OnAddRuleClicked();
	FReply OnRemoveRuleClicked();
	TSharedRef<ITableRow> GenerateListRow(LRulePtr item, const TSharedRef<STableViewBase> &ownerTable);
	void SelectionChanged(LRulePtr item, ESelectInfo::Type selectType);

public:
	TSharedPtr<FLTerrainEditorModule> lTerrainModule;

protected:
	TSharedPtr<SListView<LRulePtr>> ruleListWidget;
	TSharedPtr<SLRuleView> ruleViewWidget;
	TSharedPtr<SLSymbolSelector> brushWidget;
};

class SLRuleView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLRuleView) {}
	SLATE_ARGUMENT(LRulePtr, Rule)
	SLATE_EVENT(FOnPaint, SymbolBrush)
	SLATE_END_ARGS()

	void Construct(const FArguments& args);
	void Reconstruct(LRulePtr item);

protected:
	FOnPaint SymbolBrush;
};