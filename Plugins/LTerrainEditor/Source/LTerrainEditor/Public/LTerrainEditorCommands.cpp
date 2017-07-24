// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "LTerrainEditor.h"
#include "LTerrainEditorCommands.h"

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

void FLTerrainEditorCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "LTerrainEditor", "Execute LTerrainEditor action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
