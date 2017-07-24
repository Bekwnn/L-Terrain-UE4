#include "LTerrainEditor.h"
#include "LMapEditor.h"
#include "LMapView.h"

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

void SLMapEditor::Construct(const FArguments & InArgs)
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
				SpamTestScroll()
			]
			+ SVerticalBox::Slot()
			.Padding(2)
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("GenNewLoDButton", "+ Gen New LoD From Selected"))
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

FReply SLMapEditor::OnAddLoDClicked()
{
	//TODO
	return FReply::Handled();
}

FReply SLMapEditor::OnRemoveLoDClicked()
{
	//TODO
	return FReply::Handled();
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