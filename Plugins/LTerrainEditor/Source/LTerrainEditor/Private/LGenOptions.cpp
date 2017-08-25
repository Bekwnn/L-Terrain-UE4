#include "LTerrainEditor.h"
#include "LGenOptions.h"
#include "LTerrainGeneration.h"
#include "Kismet/GameplayStatics.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

void SLGenOptions::Construct(const FArguments & args)
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
				SNew(SButton)
				.Text(LOCTEXT("GenTerrainButton", "+ Generate Terrain"))
				.OnClicked_Raw(this, &SLGenOptions::OnGenerateClicked)
			]
		]
	];
}

//any final checks or tweaks to the lSystem before calling the terrain generation function go here
FReply SLGenOptions::OnGenerateClicked()
{
	TArray<AActor*> actors;
	UWorld* world = GEditor->GetEditorWorldContext().World();
	UGameplayStatics::GetAllActorsOfClass(world, ALandscape::StaticClass(), actors);
	ALandscape* landscape = (actors.Num() > 0)? Cast<ALandscape>(actors[0]) : nullptr;
	if (lTerrainModule->lSystem.lSystemLoDs.Num() > 0 && landscape != nullptr)
	{
		LTerrainGeneration::GenerateTerrain(lTerrainModule->lSystem, landscape);
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE