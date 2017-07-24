#include "LTerrainEditor.h"
#include "LTileEditor.h"

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

void SLTileEditor::Construct(const FArguments & InArgs)
{
	lTerrainModule = FLTerrainEditorModule::GetModule();

	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(2)
			.FillHeight(1)
			[
				SAssignNew(symbolListWidget, SListView<LSymbolPtr>)
				.ListItemsSource(&(lTerrainModule->lSystem.symbols))
				.OnGenerateRow(this, &SLTileEditor::GenerateListRow)
				.OnSelectionChanged(this, &SLTileEditor::SelectionChanged)
			]
			+ SVerticalBox::Slot()
			.Padding(2)
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("NewTileButton", "+ Add New Tile"))
				.OnClicked(FOnClicked::CreateRaw(this, &SLTileEditor::OnAddTileClicked))
			]
			+ SVerticalBox::Slot()
			.Padding(2)
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("DelTileButton", "- Delete Selection"))
				.OnClicked(FOnClicked::CreateRaw(this, &SLTileEditor::OnRemoveTileClicked))
			]
		]
		+ SHorizontalBox::Slot()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("placeholder12321", "Tile options go here"))
		]
	];
}

FReply SLTileEditor::OnAddTileClicked()
{
	LSymbolPtr newSymbol = LSymbolPtr(new LSymbol('?', "New Tile"));
	lTerrainModule->lSystem.symbols.Add(newSymbol);

	symbolListWidget->SetSelection(newSymbol);
	symbolListWidget->RequestListRefresh();

	return FReply::Handled();
}

FReply SLTileEditor::OnRemoveTileClicked()
{
	TArray<LSymbolPtr> selectedItems = symbolListWidget->GetSelectedItems();
	for (LSymbolPtr item : selectedItems)
	{
		lTerrainModule->lSystem.symbols.Remove(item);
	}

	symbolListWidget->ClearSelection();
	symbolListWidget->RequestListRefresh();

	return FReply::Handled();
}

TSharedRef<ITableRow> SLTileEditor::GenerateListRow(LSymbolPtr item, const TSharedRef<STableViewBase>& ownerTable)
{
	return SNew(STableRow<LSymbolPtr>, ownerTable)
		.Padding(2)
		[
			SNew(STextBlock)
			.Text(FText::FromString(item->name))
		];
}

void SLTileEditor::SelectionChanged(LSymbolPtr item, ESelectInfo::Type selectType)
{
	//TODO
}

#undef LOCTEXT_NAMESPACE