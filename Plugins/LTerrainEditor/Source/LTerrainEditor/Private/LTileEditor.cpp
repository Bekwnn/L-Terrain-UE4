#include "LTerrainEditor.h"
#include "LTileEditor.h"

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

void SLTileEditor::Construct(const FArguments & args)
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
				.SelectionMode(ESelectionMode::Single)
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
			SNew(SBorder)
			[
				SAssignNew(tileViewWidget, SLTileView)
				.Symbol(nullptr)
			]
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

	symbolListWidget->ClearSelection();
	symbolListWidget->RequestListRefresh();

	for (LSymbolPtr item : selectedItems)
	{
		lTerrainModule->lSystem.symbols.Remove(item);
	}

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
	tileViewWidget->Reconstruct(item);
}

void SLTileView::Construct(const FArguments & args)
{
	Reconstruct(args._Symbol);
}

void SLTileView::Reconstruct(LSymbolPtr item)
{
	if (!item.IsValid()) return;

	else
	{
		ChildSlot
		[
			SNew(SGridPanel)
			+ SGridPanel::Slot(0, 0)
			.Padding(2)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Tile Name"))
			]
			+ SGridPanel::Slot(1, 0)
			.Padding(2)
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
			+ SGridPanel::Slot(0, 1)
			.Padding(2)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Tile Symbol"))
			]
			+ SGridPanel::Slot(1, 1)
			.Padding(2)
			[
				SNew(SEditableTextBox)
				.MinDesiredWidth(200)
				.Text_Lambda([item]()->FText {
					return FText::FromString(FString::Chr(item->symbol));
				})
				.OnTextChanged_Lambda([item](FText newText) {
					FString newString = newText.ToString();
					if (newString.Len() >= 1)
						item->symbol = newString[0];
				})
			]
			+ SGridPanel::Slot(0, 2)
			.Padding(2)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Tile Image"))
			]
			+ SGridPanel::Slot(1, 2)
			.Padding(2)
			[
				SNew(SObjectPropertyEntryBox)
				.AllowedClass(UTexture2D::StaticClass())
				.OnObjectChanged_Lambda([item](FAssetData newTexture) {
					item->texture = newTexture;
				})
				.ObjectPath_Lambda([item]()->FString {
					return item->texture.ObjectPath.ToString();
				})
			]
		];
	}
}

#undef LOCTEXT_NAMESPACE