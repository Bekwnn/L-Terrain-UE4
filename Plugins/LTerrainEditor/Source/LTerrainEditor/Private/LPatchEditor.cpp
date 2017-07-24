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
				//TODO
				SpamTestScroll()
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
	//TODO
	return FReply::Handled();
}

FReply SLPatchEditor::OnRemovePatchClicked()
{
	//TODO
	return FReply::Handled();
}

TSharedRef<SScrollBox> SLPatchEditor::SpamTestScroll()
{
	TSharedRef<SScrollBox> scrollBox = SNew(SScrollBox);
	for (LPatchPtr patch : lTerrainModule->lSystem.patches)
	{
		scrollBox->AddSlot()
		[
			SNew(STextBlock)
			.Text(FText::FromString(patch->name))
		];
	}
	return scrollBox;
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