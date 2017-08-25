#include "LTerrainEditor.h"
#include "LRuleEditor.h"
#include "LMapView.h"

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

void SLRuleEditor::Construct(const FArguments & args)
{
	lTerrainModule = FLTerrainEditorModule::GetModule();

	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(2)
		.AutoWidth()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(2)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("BrushLabal", "Selected Brush:"))
			]
			+ SVerticalBox::Slot()
			.Padding(2)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(brushWidget, SLSymbolSelector)
				]
				+SHorizontalBox::Slot()
				.Padding(1)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Lambda([this]()->FText {
						return FText::FromString((brushWidget->selectedSymbol.IsValid()) ? brushWidget->selectedSymbol->name : "");
					})
				]
			]
			+ SVerticalBox::Slot()
			.Padding(2)
			.FillHeight(1)
			[
				SAssignNew(ruleListWidget, SListView<LRulePtr>)
				.ListItemsSource(&(lTerrainModule->lSystem.rules))
				.OnGenerateRow(this, &SLRuleEditor::GenerateListRow)
				.OnSelectionChanged(this, &SLRuleEditor::SelectionChanged)
				.SelectionMode(ESelectionMode::Single)
			]
			+ SVerticalBox::Slot()
			.Padding(2)
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("NewRuleButton", "+ Add New Rule"))
				.OnClicked(FOnClicked::CreateRaw(this, &SLRuleEditor::OnAddRuleClicked))
			]
			+ SVerticalBox::Slot()
			.Padding(2)
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("DelRuleButton", "- Delete Selected Rule"))
				.OnClicked(FOnClicked::CreateRaw(this, &SLRuleEditor::OnRemoveRuleClicked))
			]
		]
		+ SHorizontalBox::Slot()
		.Padding(2)
		[
			SNew(SBorder)
			[
				SAssignNew(ruleViewWidget, SLRuleView)
				.Rule(nullptr)
				.SymbolBrush_Lambda([this]()->LSymbolPtr {
					return this->brushWidget->selectedSymbol;
				})
			]
		]
	];
}

FReply SLRuleEditor::OnAddRuleClicked()
{
	LSymbolPtr firstSymbol = lTerrainModule->lSystem.GetDefaultSymbol();
	LRulePtr newRule = LRule::CreatePropegateRule(firstSymbol, firstSymbol);
	newRule->name = "New Rule";
	lTerrainModule->lSystem.rules.Add(newRule);

	ruleListWidget->SetSelection(newRule);
	ruleListWidget->RequestListRefresh();

	return FReply::Handled();
}

FReply SLRuleEditor::OnRemoveRuleClicked()
{
	TArray<LRulePtr> selectedItems = ruleListWidget->GetSelectedItems();
	for (LRulePtr item : selectedItems)
	{
		lTerrainModule->lSystem.rules.Remove(item);
	}

	ruleListWidget->ClearSelection();
	ruleListWidget->RequestListRefresh();

	return FReply::Handled();
}

TSharedRef<ITableRow> SLRuleEditor::GenerateListRow(LRulePtr item, const TSharedRef<STableViewBase>& ownerTable)
{
	return SNew(STableRow<LRulePtr>, ownerTable)
		.Padding(2)
		[
			SNew(STextBlock)
			.Text(FText::FromString(item->name))
		];
}

void SLRuleEditor::SelectionChanged(LRulePtr item, ESelectInfo::Type selectType)
{
	ruleViewWidget->Reconstruct(item);
}

void SLRuleView::Construct(const FArguments & args)
{
	SymbolBrush = args._SymbolBrush;
	Reconstruct(args._Rule);
}

void SLRuleView::Reconstruct(LRulePtr item)
{
	if (!item.IsValid()) return;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Tile Name"))
			]
			+ SHorizontalBox::Slot()
			[
				SNew(SEditableTextBox)
				.MinDesiredWidth(200)
				.Text_Lambda([item]()->FText {
					return FText::FromString(item->name);
				})
				.OnTextChanged_Lambda([item](FText newText) {
					item->name = newText.ToString();
				})
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(2)
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("MatchText1", "Matches symbol "))
				]
				+ SHorizontalBox::Slot()
				.Padding(2)
				.AutoWidth()
				[
					SNew(SLSymbolSelector)
					.StartSymbol_Lambda([item]()->LSymbolPtr {
						return item->matchVal;
					})
					.OnSelectionClose_Lambda([item](LSymbolPtr selectedSymbol) {
						item->matchVal = selectedSymbol;
					})
				]
				+ SHorizontalBox::Slot()
				.Padding(2)
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("MatchText2", " and replaces with:"))
				]
			]
			+ SVerticalBox::Slot()
			[
				SNew(SLMapView)
				.Map(item->replacementVals)
				.SymbolBrush(SymbolBrush)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(FText::FromString("Require Matched Neighbors"))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SCheckBox)
					.IsChecked_Lambda([item]()->ECheckBoxState {
						return (item->bMatchNeighbors)? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
					.OnCheckStateChanged_Lambda([item](ECheckBoxState state) {
						item->bMatchNeighbors = (state == ECheckBoxState::Checked) ? true : false;
					})
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SLMapView)
				.Map(item->matchNeighborsMap)
				.Visibility_Lambda([item]()->EVisibility {
					return (item->bMatchNeighbors)? EVisibility::Visible : EVisibility::Collapsed;
				})
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE