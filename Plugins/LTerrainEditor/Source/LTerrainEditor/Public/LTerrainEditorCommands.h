// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateBasics.h"
#include "LTerrainEditorStyle.h"

class FLTerrainEditorCommands : public TCommands<FLTerrainEditorCommands>
{
public:

	FLTerrainEditorCommands()
		: TCommands<FLTerrainEditorCommands>(TEXT("LTerrainEditor"), NSLOCTEXT("Contexts", "LTerrainEditor", "LTerrainEditor Plugin"), NAME_None, FLTerrainEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
