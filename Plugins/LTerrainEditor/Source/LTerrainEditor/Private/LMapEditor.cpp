#include "LTerrainEditor.h"
#include "LMapEditor.h"
#include "LMapView.h"

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

void SLMapEditor::Construct(const FArguments & args)
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
				SAssignNew(lodListWidget, SListView<LSymbol2DMapPtr>)
				.ListItemsSource(&(lTerrainModule->lSystem.lSystemLoDs))
				.OnGenerateRow(this, &SLMapEditor::GenerateListRow)
				.OnSelectionChanged(this, &SLMapEditor::SelectionChanged)
				.SelectionMode(ESelectionMode::Single)
			]
			+ SVerticalBox::Slot()
			.Padding(2)
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("GenNewLoDButton", "+ Gen New LoD"))
				.OnClicked(FOnClicked::CreateRaw(this, &SLMapEditor::OnAddLoDClicked))
			]
			+ SVerticalBox::Slot()
			.Padding(2)
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("RemoveLoDButton", "- Remove Selected LoD and Below"))
				.OnClicked(FOnClicked::CreateRaw(this, &SLMapEditor::OnRemoveLoDClicked))
			]
		]
		+ SHorizontalBox::Slot()
		.Padding(2)
		[
			SNew(SBorder)
			.Padding(2)
			[
				SNew(SLMapView)
			]
		]
	];
}

//generates new LoD from highest current and adds it
FReply SLMapEditor::OnAddLoDClicked()
{
	if (lTerrainModule->lSystem.lSystemLoDs.Num() > 4) return FReply::Handled();
	LSymbol2DMapPtr highestLoD = lTerrainModule->lSystem.lSystemLoDs[lTerrainModule->lSystem.lSystemLoDs.Num() - 1];
	LSymbol2DMapPtr newLoD = lTerrainModule->lSystem.IterateLString(highestLoD);
	lTerrainModule->lSystem.lSystemLoDs.Add(newLoD);

	lodListWidget->SetSelection(newLoD);
	lodListWidget->RequestListRefresh();

	return FReply::Handled();
}

FReply SLMapEditor::OnRemoveLoDClicked()
{
	TArray<LSymbol2DMapPtr> selectedLoD = lodListWidget->GetSelectedItems();
	int idx;
	lTerrainModule->lSystem.lSystemLoDs.Find(selectedLoD[0], idx);
	if (idx == 0) return FReply::Handled(); //Never remove LoD 0

	int count = lTerrainModule->lSystem.lSystemLoDs.Num();
	for (int i = idx; i < count; ++i)
	{
		lTerrainModule->lSystem.lSystemLoDs.Pop(false);
	}
	lTerrainModule->lSystem.lSystemLoDs.Shrink();

	lodListWidget->ClearSelection();
	lodListWidget->RequestListRefresh();

	return FReply::Handled();
}

TSharedRef<ITableRow> SLMapEditor::GenerateListRow(LSymbol2DMapPtr item, const TSharedRef<STableViewBase>& ownerTable)
{
	int idx;
	lTerrainModule->lSystem.lSystemLoDs.Find(item, idx);

	return SNew(STableRow<LSymbolPtr>, ownerTable)
		.Padding(2)
		[
			SNew(STextBlock)
			.Text(FText::FromString("LoD " + FString::FromInt(idx)))
		];
}

void SLMapEditor::SelectionChanged(LSymbol2DMapPtr item, ESelectInfo::Type selectType)
{
	//TODO
}

TSharedRef<SHorizontalBox> SLMapEditor::NewBrushBox()
{
	return
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
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
		+SHorizontalBox::Slot()
		.Padding(1)
		.VAlign(EVerticalAlignment::VAlign_Center)
		[
			//TODO
			SNew(STextBlock)
			.Text(LOCTEXT("BrushBox", "TODO: brush name here"))
		];
}

TSharedRef<SScrollBox> SLMapEditor::SpamTestScroll()
{
	TSharedRef<SScrollBox> scrollBox = SNew(SScrollBox);
	for (int i = 0; i < lTerrainModule->lSystem.lSystemLoDs.Num(); ++i)
	{
		FString&& display = "LoD " + FString::FromInt(i);
		scrollBox->AddSlot()
		[
			SNew(STextBlock)
			.Text(FText::FromString(display))
		];
	}
	return scrollBox;
}

TSharedRef<SWidget> SLMapEditor::BrushMenuTest()
{
	TSharedRef<SVerticalBox> vertBox = SNew(SVerticalBox);
	for (int i = 0; i < 5; ++i)
	{
		TSharedRef<SHorizontalBox> horBox = SNew(SHorizontalBox);
		for (int j = 0; j < 3; ++j)
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