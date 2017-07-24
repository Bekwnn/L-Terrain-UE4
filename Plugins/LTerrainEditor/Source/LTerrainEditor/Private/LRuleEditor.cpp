#include "LTerrainEditor.h"
#include "LRuleEditor.h"

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

void SLRuleEditor::Construct(const FArguments & InArgs)
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
				NewBrushBox()
			]
			+ SVerticalBox::Slot()
			.Padding(2)
			.FillHeight(1)
			[
				//TODO
				SAssignNew(ruleListWidget, SListView<LRulePtr>)
				.ListItemsSource(&(lTerrainModule->lSystem.rules))
				.OnGenerateRow(this, &SLRuleEditor::GenerateListRow)
				.OnSelectionChanged(this, &SLRuleEditor::SelectionChanged)
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
			.Padding(2)
			[
				SNew(SBox)
			]
		]
	];
}

FReply SLRuleEditor::OnAddRuleClicked()
{
	LSymbolPtr firstSymbol = (lTerrainModule->lSystem.symbols.Num() > 0) ? lTerrainModule->lSystem.symbols[0] : nullptr;
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
	//TODO
}

TSharedRef<SHorizontalBox> SLRuleEditor::NewBrushBox()
{
	//TODO
	return
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(1)
		.AutoWidth()
		[
			SNew(SComboButton)
			.ButtonContent()
			[
				SNew(SBox)
				.MinDesiredHeight(32)
				.MinDesiredWidth(32)
				[
					SNew(SImage)
					.ColorAndOpacity(FLinearColor::Blue)
				]
			]
			.MenuContent()
			[
				BrushMenuTest()
			]
		]
		+ SHorizontalBox::Slot()
		.Padding(1)
		.VAlign(EVerticalAlignment::VAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("BrushBox2", "TODO: brush name here"))
		];
}

TSharedRef<SWidget> SLRuleEditor::BrushMenuTest()
{
	//TODO
	TSharedRef<SVerticalBox> vertBox = SNew(SVerticalBox);
	for (int i = 0; i < 5; ++i)
	{
		TSharedRef<SHorizontalBox> horBox = SNew(SHorizontalBox);
		for (int j = 0; j < 3; ++j) //Drop-down is 3 brushes selections wide
		{
			horBox->AddSlot()
			[
				SNew(SBox)
				.MinDesiredHeight(32)
				.MinDesiredWidth(32)
				.Padding(1)
				[
					SNew(SImage)
					.ColorAndOpacity(FLinearColor::Blue)
				]
			];
		}
		vertBox->AddSlot()
		[
			horBox
		];
	}
	return vertBox;
}

#undef LOCTEXT_NAMESPACE