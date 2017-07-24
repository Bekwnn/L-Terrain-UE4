#include "LTerrainEditor.h"
#include "LGenOptions.h"

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

void SLGenOptions::Construct(const FArguments & InArgs)
{
	lTerrainModule = FLTerrainEditorModule::GetModule();

	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Center)
		.AutoWidth()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("placeholder69049", "Some options go here."))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("GenTerrainButton", "+ Generate Terrain"))
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE