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
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
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

void SLRuleView::Construct(const FArguments & args)
{
	Reconstruct(args._Rule);
}

void SLRuleView::Reconstruct(LRulePtr item)
{
	if (!item.IsValid()) return;

	TSharedRef<SUniformGridPanel> ruleGridPanel = SNew(SUniformGridPanel)
		.SlotPadding(2)
		.MinDesiredSlotWidth(64)
		.MinDesiredSlotHeight(64);

	for (int i = 0; i < item->replacementVals->Num(); ++i)
	{
		for (int j = 0; j < (*(item->replacementVals))[i].Num(); ++j)
		{
			LSymbolPtr symbol = (*(item->replacementVals))[i][j];
			TSharedPtr<SBorder> slotWidget;

			if (symbol->texture.IsValid()) //if texture is set
			{
				slotWidget = SNew(SBorder)
				[
					SNew(SBox)
					[
						SNew(SImage)
						.ColorAndOpacity(FLinearColor::Blue)
					]
					/*//TODO
					.Image_Lambda([item, i, j]()->FSlateBrush {
						LSymbolPtr symb = (*(item->replacementVals))[i][j];
						FSlateBrush brush = FSlateBrush();
						brush.SetResourceObject(symb->texture.GetAsset());
						return brush;
					})
					*/
				];
			}
			else
			{
				slotWidget = SNew(SBorder) // if no texture is set
				[
					SNew(SBox)
					[
						SNew(SImage)
						.ColorAndOpacity(FLinearColor::Blue)
					]
					/*//TODO
					SNew(STextBlock)
					.Text_Lambda([item, i, j]()->FText {
						LSymbolPtr symb = (*(item->replacementVals))[i][j];
						return FText::FromString(FString::Chr(symb->symbol));
					})
					*/
				];
			}

			ruleGridPanel->AddSlot(j, i)
			[
				slotWidget.ToSharedRef()
			];
		}
	}

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
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE