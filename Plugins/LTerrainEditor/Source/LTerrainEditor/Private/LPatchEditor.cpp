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
		+ SVerticalBox::Slot()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SAssignNew(noiseListWidget, SListView<LNoisePtr>)
				.ListItemsSource(&item->noiseMaps)
				.OnGenerateRow(this, &SLPatchView::GenerateNoiseListRow)
				.OnSelectionChanged(this, &SLPatchView::NoiseSelectionChanged)
				.SelectionMode(ESelectionMode::Single)
			]
			+ SHorizontalBox::Slot()
			[
				SNew(SLNoiseView)
			]
		]
		+ SVerticalBox::Slot()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SAssignNew(groundTexListWidget, SListView<LGroundTexturePtr>)
				.ListItemsSource(&item->groundTextures)
				.OnGenerateRow(this, &SLPatchView::GenerateGroundTexListRow)
				.OnSelectionChanged(this, &SLPatchView::GroundTexSelectionChanged)
				.SelectionMode(ESelectionMode::Single)
			]
			+ SHorizontalBox::Slot()
			[
				SNew(SLGroundTexView)
			]
		]
		+ SVerticalBox::Slot()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SAssignNew(scatterListWidget, SListView<LObjectScatterPtr>)
				.ListItemsSource(&item->objectScatters)
				.OnGenerateRow(this, &SLPatchView::GenerateScatterListRow)
				.OnSelectionChanged(this, &SLPatchView::ScatterSelectionChanged)
				.SelectionMode(ESelectionMode::Single)
			]
			+ SHorizontalBox::Slot()
			[
				SNew(SLScatterView)
			]
		]
	];
}

FReply SLPatchView::OnNoiseAdded()
{
	//TODO

	
	return FReply::Handled();
}

FReply SLPatchView::OnNoiseRemoved()
{
	//TODO

	return FReply::Handled();
}

TSharedRef<ITableRow> SLPatchView::GenerateNoiseListRow(LNoisePtr item, const TSharedRef<STableViewBase>& ownerTable)
{
	FString itemName = "";
	switch (item->GetNoiseType())
	{
	case ENoiseType::WHITE:
		itemName += "White Noise";
		break;
	case ENoiseType::RED:
		itemName += "Red Noise";
		break;
	case ENoiseType::BLUE:
		itemName += "Blue Noise";
		break;
	case ENoiseType::PERLIN:
		itemName += "Perlin Noise";
		break;
	default:
		break;
	}

	itemName += FString::Printf(TEXT(" F%.2f A%.2f"), item->frequency, item->amplitude);

	return SNew(STableRow<LNoisePtr>, ownerTable)
		.Padding(2)
		[
			SNew(STextBlock)
			.Text(FText::FromString(itemName))
		];
}

void SLPatchView::NoiseSelectionChanged(LNoisePtr item, ESelectInfo::Type selectType)
{
	noiseView->Reconstruct(item);
}

FReply SLPatchView::OnGroundTexAdded()
{
	//TODO

	return FReply::Handled();
}

FReply SLPatchView::OnGroundTexRemoved()
{
	//TODO

	return FReply::Handled();
}

TSharedRef<ITableRow> SLPatchView::GenerateGroundTexListRow(LGroundTexturePtr item, const TSharedRef<STableViewBase> &ownerTable)
{
	return SNew(STableRow<LGroundTexturePtr>, ownerTable)
		.Padding(2)
		[
			SNew(STextBlock)
			.Text(FText::FromString(item->name))
		];
}

void SLPatchView::GroundTexSelectionChanged(LGroundTexturePtr item, ESelectInfo::Type selectType)
{
	groundTexView->Reconstruct(item);
}

FReply SLPatchView::OnScatterAdded()
{
	//TODO

	return FReply::Handled();
}

FReply SLPatchView::OnScatterRemoved()
{
	//TODO

	return FReply::Handled();
}

TSharedRef<ITableRow> SLPatchView::GenerateScatterListRow(LObjectScatterPtr item, const TSharedRef<STableViewBase>& ownerTable)
{
	return SNew(STableRow<LObjectScatterPtr>, ownerTable)
		.Padding(2)
		[
			SNew(STextBlock)
			.Text(FText::FromString(item->name))
		];
}

void SLPatchView::ScatterSelectionChanged(LObjectScatterPtr item, ESelectInfo::Type selectType)
{
	scatterView->Reconstruct(item);
}

void SLNoiseView::Construct(const FArguments& args)
{
	Reconstruct(args._Noise);
}

void SLNoiseView::Reconstruct(LNoisePtr item)
{
	if (item.IsValid()) return;

	ChildSlot
	[
		SNew(STextBlock)
		.Text(FText::FromString("Placeholder noise text."))
	];
		/*
		+ SVerticalBox::Slot()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PatchHeightMin", "Height Min:"))
			]
			+ SHorizontalBox::Slot()
			[
				SNew(SSpinBox<float>)//TODO: make do something
			]
			+ SHorizontalBox::Slot()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PatchHeightMax", "Max:"))
			]
			+ SHorizontalBox::Slot()
			[
				SNew(SSpinBox<float>)//TODO: make do something
			]
		]
		*/
}

void SLGroundTexView::Construct(const FArguments& args)
{
	Reconstruct(args._GroundTexture);
}

void SLGroundTexView::Reconstruct(LGroundTexturePtr item)
{
	if (item.IsValid()) return;

	ChildSlot
	[
		SNew(STextBlock)
		.Text(FText::FromString("Placeholder ground text."))
	];
}

void SLScatterView::Construct(const FArguments& args)
{
	Reconstruct(args._ObjectScatter);
}

void SLScatterView::Reconstruct(LObjectScatterPtr item)
{
	if (item.IsValid()) return;

	ChildSlot
	[
		SNew(STextBlock)
		.Text(FText::FromString("Placeholder scatter text."))
	];
}

#undef LOCTEXT_NAMESPACE