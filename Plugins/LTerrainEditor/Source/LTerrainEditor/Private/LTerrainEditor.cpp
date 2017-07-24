// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "LTerrainEditor.h"

#include "LTerrainEditorStyle.h"
#include "LTerrainEditorCommands.h"

#include "LMapEditor.h"
#include "LRuleEditor.h"
#include "LTileEditor.h"
#include "LPatchEditor.h"
#include "LGenOptions.h"

#include "LevelEditor.h"
#include "SharedPointer.h"


static const FName MapEditorTabName("LMapEditor");
static const FName RuleEditorTabName("LRuleEditor");
static const FName TileEditorTabName("LTileEditor");
static const FName PatchEditorTabName("LPatchEditor");
static const FName GenOptionsTabName("LGenOptions");

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

void FLTerrainEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FLTerrainEditorStyle::Initialize();
	FLTerrainEditorStyle::ReloadTextures();

	FLTerrainEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FLTerrainEditorCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FLTerrainEditorModule::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FLTerrainEditorModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FLTerrainEditorModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}

	FGlobalTabmanager::Get()->RegisterTabSpawner(
		MapEditorTabName,
		FOnSpawnTab::CreateRaw(this, &FLTerrainEditorModule::SpawnMapEditorTab));

	FGlobalTabmanager::Get()->RegisterTabSpawner(
		RuleEditorTabName,
		FOnSpawnTab::CreateRaw(this, &FLTerrainEditorModule::SpawnRuleEditorTab));

	FGlobalTabmanager::Get()->RegisterTabSpawner(
		TileEditorTabName,
		FOnSpawnTab::CreateRaw(this, &FLTerrainEditorModule::SpawnTileEditorTab));

	FGlobalTabmanager::Get()->RegisterTabSpawner(
		PatchEditorTabName,
		FOnSpawnTab::CreateRaw(this, &FLTerrainEditorModule::SpawnPatchEditorTab));

	FGlobalTabmanager::Get()->RegisterTabSpawner(
		GenOptionsTabName,
		FOnSpawnTab::CreateRaw(this, &FLTerrainEditorModule::SpawnGenOptionsTab));

	lSystem = LSystem();

	lSystem.Reset();
}

void FLTerrainEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FGlobalTabmanager::Get()->UnregisterTabSpawner(MapEditorTabName);
	FGlobalTabmanager::Get()->UnregisterTabSpawner(RuleEditorTabName);
	FGlobalTabmanager::Get()->UnregisterTabSpawner(TileEditorTabName);
	FGlobalTabmanager::Get()->UnregisterTabSpawner(PatchEditorTabName);
	FGlobalTabmanager::Get()->UnregisterTabSpawner(GenOptionsTabName);

	FLTerrainEditorStyle::Shutdown();

	FLTerrainEditorCommands::Unregister();
}

TSharedPtr<FLTerrainEditorModule> FLTerrainEditorModule::GetModule()
{
	return StaticCastSharedPtr<FLTerrainEditorModule, IModuleInterface>(FModuleManager::Get().GetModule("LTerrainEditor"));
}

void FLTerrainEditorModule::PluginButtonClicked()
{
	TSharedRef<SDockTab> MapEditor = FGlobalTabmanager::Get()->InvokeTab(MapEditorTabName);
	TSharedRef<SDockTab> RuleEditor = FGlobalTabmanager::Get()->InvokeTab(RuleEditorTabName);
	TSharedRef<SDockTab> TileEditor = FGlobalTabmanager::Get()->InvokeTab(TileEditorTabName);
	TSharedRef<SDockTab> PatchEditor = FGlobalTabmanager::Get()->InvokeTab(PatchEditorTabName);
	TSharedRef<SDockTab> GenOptions = FGlobalTabmanager::Get()->InvokeTab(GenOptionsTabName);
}

TSharedRef<SDockTab> FLTerrainEditorModule::SpawnMapEditorTab(const FSpawnTabArgs & SpawnTabArgs)
{
	return
		SNew(SDockTab)
			.Label(LOCTEXT("LMapEditor", "Map Editor"))
			.TabRole(ETabRole::MajorTab)
			.ContentPadding(2)
		[
			SNew(SLMapEditor)
		];
}

TSharedRef<SDockTab> FLTerrainEditorModule::SpawnTileEditorTab(const FSpawnTabArgs & SpawnTabArgs)
{
	return
		SNew(SDockTab)
			.Label(LOCTEXT("LTileEditor", "Tile Editor"))
			.TabRole(ETabRole::MajorTab)
			.ContentPadding(2)
		[
			SNew(SLTileEditor)
		];
}

TSharedRef<SDockTab> FLTerrainEditorModule::SpawnRuleEditorTab(const FSpawnTabArgs & SpawnTabArgs)
{
	return
		SNew(SDockTab)
			.Label(LOCTEXT("LRuleEditor", "Rule Editor"))
			.TabRole(ETabRole::MajorTab)
			.ContentPadding(2)
		[
			SNew(SLRuleEditor)
		];
}

TSharedRef<SDockTab> FLTerrainEditorModule::SpawnPatchEditorTab(const FSpawnTabArgs & SpawnTabArgs)
{
	return
		SNew(SDockTab)
		.Label(LOCTEXT("LPatchEditor", "Patch Editor"))
		.TabRole(ETabRole::MajorTab)
		.ContentPadding(2)
		[
			SNew(SLPatchEditor)
		];
}

TSharedRef<SDockTab> FLTerrainEditorModule::SpawnGenOptionsTab(const FSpawnTabArgs & SpawnTabArgs)
{
	return
		SNew(SDockTab)
		.Label(LOCTEXT("LGenOptions", "Generation Options"))
		.TabRole(ETabRole::MajorTab)
		.ContentPadding(2)
		[
			SNew(SLGenOptions)
		];
}

void FLTerrainEditorModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FLTerrainEditorCommands::Get().PluginAction);
}

void FLTerrainEditorModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FLTerrainEditorCommands::Get().PluginAction);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLTerrainEditorModule, LTerrainEditor)