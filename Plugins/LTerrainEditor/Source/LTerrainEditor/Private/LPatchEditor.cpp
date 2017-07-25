#include "LTerrainEditor.h"
#include "LPatchEditor.h"

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

void SLPatchEditor::Construct(const FArguments & InArgs)
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
					SpawnMatchWidget()
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
	//TODO
}

TSharedRef<SHorizontalBox> SLPatchEditor::SpawnMatchWidget()
{
	return
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
		+SHorizontalBox::Slot()
		.Padding(2)
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("MatchText2", " and replaces with patch contents:"))
		];
}

#undef LOCTEXT_NAMESPACE