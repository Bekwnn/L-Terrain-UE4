// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyEditorModule.h"

#include "Engine/Texture2D.h"
#include "AssetData.h"

#include "LSystem.h"

class FToolBarBuilder;
class FMenuBuilder;

class FLTerrainEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static TSharedPtr<FLTerrainEditorModule> GetModule();
	
	/** This function will be bound to Command. */
	void PluginButtonClicked();
	TSharedRef<SDockTab> SpawnMapEditorTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> SpawnTileEditorTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> SpawnRuleEditorTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> SpawnPatchEditorTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> SpawnGenOptionsTab(const FSpawnTabArgs& SpawnTabArgs);
	
	LSystem lSystem;

private:
	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
	bool bHasBeenOpened;
};