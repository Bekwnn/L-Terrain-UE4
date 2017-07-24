#pragma once
#include "LTerrainEditor.h"
#include "SlateCore.h"
#include "Framework/Layout/ScrollyZoomy.h"
#include "Framework/Docking/TabManager.h"

//Spawns the map editor tab and ui

class SLMapView :public IScrollableZoomable, public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLMapView) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	//IScrollableZoomable interface TODO
	virtual bool ScrollBy(const FVector2D& Offset) override { return false; }
	virtual bool ZoomBy(const float Amount) override { return false; }

public:
	TSharedPtr<FLTerrainEditorModule> lTerrainModule;
};