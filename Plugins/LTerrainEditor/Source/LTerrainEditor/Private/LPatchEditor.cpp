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

	//save the patch argument for later use
	patch = item;
	
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
		.Padding(2)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PatchHeightMin", "Height Min:"))
			]
			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SSpinBox<float>)
				.MinValue(-250.f)
				.MaxValue(250.f)
				.MinDesiredWidth(80.f)
				.Value_Lambda([item]()->float {
					return item->minHeight;
				})
				.OnValueChanged_Lambda([item](float val) {
					item->minHeight = FMath::Clamp(val, -250.f, item->maxHeight);
				})
			]
			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PatchHeightMax", "Max:"))
			]
			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SSpinBox<float>)
				.MinValue(-250.f)
				.MaxValue(250.f)
				.MinDesiredWidth(80.f)
				.Value_Lambda([item]()->float {
					return item->maxHeight;
				})
				.OnValueChanged_Lambda([item](float val) {
					item->maxHeight = FMath::Clamp(val, item->minHeight, 250.f);
				})
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SmoothNeighbors", "Smooth Height to Neighbors"))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([item]()->ECheckBoxState {
					return (item->bHeightMatch)? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
				.OnCheckStateChanged_Lambda([item](ECheckBoxState checkstate) {
					item->bHeightMatch = (checkstate == ECheckBoxState::Checked);
				})
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2)
			[
				SNew(STextBlock)
				.IsEnabled_Lambda([item]()->bool {
					return item->bHeightMatch;
				})
				.Text(LOCTEXT("SmoothNeighborsFactor", " Smoothing Strength:"))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2)
			[
				SNew(SSpinBox<float>)
				.IsEnabled_Lambda([item]()->bool {
					return item->bHeightMatch;
				})
				.MinValue(0.f)
				.MaxValue(1.f)
				.MinDesiredWidth(80.f)
				.Value_Lambda([item]()->float {
					return item->heightSmoothFactor;
				})
				.OnValueChanged_Lambda([item](float val) {
					item->heightSmoothFactor = val;
				})
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(2)
				.FillHeight(1)
				[
					SAssignNew(noiseListWidget, SListView<LNoisePtr>)
					.ListItemsSource(&item->noiseMaps)
					.OnGenerateRow(this, &SLPatchView::GenerateNoiseListRow)
					.OnSelectionChanged(this, &SLPatchView::NoiseSelectionChanged)
					.SelectionMode(ESelectionMode::Single)
				]
				+ SVerticalBox::Slot()
				.Padding(2)
				.AutoHeight()
				[
					SNew(SButton)
					.Text(LOCTEXT("NoiseAdd", "+ Add Noise"))
					.OnClicked(FOnClicked::CreateRaw(this, &SLPatchView::OnNoiseAdded))
				]
				+ SVerticalBox::Slot()
				.Padding(2)
				.AutoHeight()
				[
					SNew(SButton)
					.Text(LOCTEXT("NoiseDel", "- Del Selected Noise"))
					.OnClicked(FOnClicked::CreateRaw(this, &SLPatchView::OnNoiseRemoved))
				]
			]
			+ SHorizontalBox::Slot()
			.Padding(2)
			[
				SAssignNew(noiseView, SLNoiseView)
				.Noise(nullptr)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(2)
				.FillHeight(1)
				[
					SAssignNew(groundTexListWidget, SListView<LGroundTexturePtr>)
					.ListItemsSource(&item->groundTextures)
					.OnGenerateRow(this, &SLPatchView::GenerateGroundTexListRow)
					.OnSelectionChanged(this, &SLPatchView::GroundTexSelectionChanged)
					.SelectionMode(ESelectionMode::Single)
				]
				+ SVerticalBox::Slot()
				.Padding(2)
				.AutoHeight()
				[
					SNew(SButton)
					.Text(LOCTEXT("TexAdd", "+ Add Texture"))
					.OnClicked(FOnClicked::CreateRaw(this, &SLPatchView::OnGroundTexAdded))
				]
				+ SVerticalBox::Slot()
				.Padding(2)
				.AutoHeight()
				[
					SNew(SButton)
					.Text(LOCTEXT("TexDel", "- Del Selected Texture"))
					.OnClicked(FOnClicked::CreateRaw(this, &SLPatchView::OnGroundTexRemoved))
				]
			]
			+ SHorizontalBox::Slot()
			.Padding(2)
			[
				SAssignNew(groundTexView, SLGroundTexView)
				.GroundTexture(nullptr)
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
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(2)
				.FillHeight(1)
				[
					SAssignNew(scatterListWidget, SListView<LObjectScatterPtr>)
					.ListItemsSource(&item->objectScatters)
					.OnGenerateRow(this, &SLPatchView::GenerateScatterListRow)
					.OnSelectionChanged(this, &SLPatchView::ScatterSelectionChanged)
					.SelectionMode(ESelectionMode::Single)
				]
				+ SVerticalBox::Slot()
				.Padding(2)
				.AutoHeight()
				[
					SNew(SButton)
					.Text(LOCTEXT("ScatterAdd", "+ Add Scatter"))
					.OnClicked(FOnClicked::CreateRaw(this, &SLPatchView::OnScatterAdded))
				]
				+ SVerticalBox::Slot()
				.Padding(2)
				.AutoHeight()
				[
					SNew(SButton)
					.Text(LOCTEXT("ScatterDel", "- Del Selected Scatter"))
					.OnClicked(FOnClicked::CreateRaw(this, &SLPatchView::OnScatterRemoved))
				]
			]
			+ SHorizontalBox::Slot()
			.Padding(2)
			[
				SAssignNew(scatterView, SLScatterView)
				.ObjectScatter(nullptr)
			]
		]
	];
}

FReply SLPatchView::OnNoiseAdded()
{
	LNoisePtr newNoise = LNoisePtr(new LNoise(ENoiseType::WHITE));
	newNoise->frequency = 1.f;
	newNoise->amplitude = 2.f;
	patch->noiseMaps.Add(newNoise);

	noiseListWidget->SetSelection(newNoise);
	noiseListWidget->RequestListRefresh();
	
	return FReply::Handled();
}

FReply SLPatchView::OnNoiseRemoved()
{
	TArray<LNoisePtr> selectedItems = noiseListWidget->GetSelectedItems();
	for (LNoisePtr item : selectedItems)
	{
		patch->noiseMaps.Remove(item);
	}

	noiseListWidget->ClearSelection();
	noiseListWidget->RequestListRefresh();

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
	case ENoiseType::PINK:
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
	LGroundTexturePtr newGroundTex = LGroundTexturePtr(new LGroundTexture());
	newGroundTex->name = "New Ground Texture";
	patch->groundTextures.Add(newGroundTex);

	groundTexListWidget->SetSelection(newGroundTex);
	groundTexListWidget->RequestListRefresh();

	return FReply::Handled();
}

FReply SLPatchView::OnGroundTexRemoved()
{
	TArray<LGroundTexturePtr> selectedItems = groundTexListWidget->GetSelectedItems();
	for (LGroundTexturePtr item : selectedItems)
	{
		patch->groundTextures.Remove(item);
	}

	groundTexListWidget->ClearSelection();
	groundTexListWidget->RequestListRefresh();

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
	LObjectScatterPtr newScatter = LObjectScatterPtr(new LObjectScatter());
	newScatter->name = "New Object Scatter";
	patch->objectScatters.Add(newScatter);

	scatterListWidget->SetSelection(newScatter);
	scatterListWidget->RequestListRefresh();

	return FReply::Handled();
}

FReply SLPatchView::OnScatterRemoved()
{
	TArray<LObjectScatterPtr> selectedItems = scatterListWidget->GetSelectedItems();
	for (LObjectScatterPtr item : selectedItems)
	{
		patch->objectScatters.Remove(item);
	}

	scatterListWidget->ClearSelection();
	scatterListWidget->RequestListRefresh();

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

//used as text fields for enum selection
TArray<TSharedPtr<FString>> SLNoiseView::noiseNames = {
	TSharedPtr<FString>(new FString("White Noise")),
	TSharedPtr<FString>(new FString("Pink Noise")),
	TSharedPtr<FString>(new FString("Blue Noise")),
	TSharedPtr<FString>(new FString("Perlin Noise"))
};

void SLNoiseView::Construct(const FArguments& args)
{
	Reconstruct(args._Noise);
}

void SLNoiseView::Reconstruct(LNoisePtr item)
{
	if (!item.IsValid()) return;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(2)
		.AutoHeight()
		[
			SNew(STextComboBox)
			.InitiallySelectedItem(noiseNames[(int)item->GetNoiseType()])
			.OptionsSource(&noiseNames)
			.OnSelectionChanged_Lambda([item, this](TSharedPtr<FString> string, ESelectInfo::Type selectType) {
				ENoiseType newNoiseType = (ENoiseType)noiseNames.Find(string);
				*item = LNoise(newNoiseType);
				item->frequency = 1.f;
				item->amplitude = 2.f;
				this->Reconstruct(item);
			})
		]
		+ SVerticalBox::Slot()
		.Padding(2)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PatchFrequency", "Frequency:"))
			]
			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SSpinBox<float>)
				.MinDesiredWidth(80.f)
				.MinValue(0.001f)
				.MaxValue(10.f)
				.Value_Lambda([item]()->float {
					return item->frequency;
				})
				.OnValueChanged_Lambda([item](float val) {
					item->frequency = val;
				})
			]
			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PatchAmplitude", " Amplitude:"))
			]
			+ SHorizontalBox::Slot()
			.Padding(2)
			.AutoWidth()
			[
				SNew(SSpinBox<float>)
				.MinDesiredWidth(80.f)
				.MinValue(0.f)
				.MaxValue(10.f)
				.Value_Lambda([item]()->float {
					return item->amplitude;
				})
				.OnValueChanged_Lambda([item](float val) {
					item->amplitude = val;
				})
			]
		]
	];
}

void SLGroundTexView::Construct(const FArguments& args)
{
	Reconstruct(args._GroundTexture);
}

void SLGroundTexView::Reconstruct(LGroundTexturePtr item)
{
	if (!item.IsValid()) return;

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
	if (!item.IsValid()) return;

	ChildSlot
	[
		SNew(STextBlock)
		.Text(FText::FromString("Placeholder scatter text."))
	];
}

#undef LOCTEXT_NAMESPACE