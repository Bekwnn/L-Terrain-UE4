#include "LTerrainEditor.h"
#include "LPatchEditor.h"
#include "LSymbolSelector.h"

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

void SLPatchEditor::Construct(const FArguments & args)
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
			.FillHeight(1)
			[
				SAssignNew(patchListWidget, SListView<LPatchPtr>)
				.ListItemsSource(&(lTerrainModule->lSystem.patches))
				.OnGenerateRow(this, &SLPatchEditor::GenerateListRow)
				.OnSelectionChanged(this, &SLPatchEditor::SelectionChanged)
				.SelectionMode(ESelectionMode::Single)
			]
			+ SVerticalBox::Slot()
			.Padding(2)
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("NewPatchButton", "+ Add New Patch"))
				.OnClicked(FOnClicked::CreateRaw(this, &SLPatchEditor::OnAddPatchClicked))
			]
			+ SVerticalBox::Slot()
			.Padding(2)
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("DelPatchButton", "- Delete Selected Patch"))
				.OnClicked(FOnClicked::CreateRaw(this, &SLPatchEditor::OnRemovePatchClicked))
			]
		]
		+ SHorizontalBox::Slot()
		.Padding(2)
		[
			SNew(SBorder)
			.Padding(2)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(patchViewWidget, SLPatchView)
					.Patch(nullptr)
				]
			]
		]
	];
}

FReply SLPatchEditor::OnAddPatchClicked()
{
	LPatchPtr newPatch = LPatchPtr(new LPatch());
	newPatch->name = "New Patch";
	newPatch->matchVal = lTerrainModule->lSystem.GetDefaultSymbol();
	newPatch->minHeight = 1;
	newPatch->maxHeight = 10;
	lTerrainModule->lSystem.patches.Add(newPatch);

	patchListWidget->SetSelection(newPatch);
	patchListWidget->RequestListRefresh();

	return FReply::Handled();
}

FReply SLPatchEditor::OnRemovePatchClicked()
{
	TArray<LPatchPtr> selectedItems = patchListWidget->GetSelectedItems();
	for (LPatchPtr item : selectedItems)
	{
		lTerrainModule->lSystem.patches.Remove(item);
	}

	patchListWidget->ClearSelection();
	patchListWidget->RequestListRefresh();

	return FReply::Handled();
}

TSharedRef<ITableRow> SLPatchEditor::GenerateListRow(LPatchPtr item, const TSharedRef<STableViewBase>& ownerTable)
{
	return SNew(STableRow<LPatchPtr>, ownerTable)
		.Padding(2)
		[
			SNew(STextBlock)
			.Text(FText::FromString(item->name))
		];
}

void SLPatchEditor::SelectionChanged(LPatchPtr item, ESelectInfo::Type selectType)
{
	patchViewWidget->Reconstruct(item);
}

void SLPatchView::Construct(const FArguments & args)
{
	Reconstruct(args._Patch);
}

void SLPatchView::Reconstruct(LPatchPtr item)
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
			.Padding(2)
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(FText::FromString("Patch Name"))
			]
			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
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
			+SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("MatchText2", " and replaces with patch contents:"))
			]
		]
		//TODO: patch parameter UI
	];
}

#undef LOCTEXT_NAMESPACE