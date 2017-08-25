#pragma once
#include "LTerrainEditor.h"

class SLSymbolBox : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLSymbolBox) {}
	SLATE_ATTRIBUTE(LSymbolPtr, Symbol)
	SLATE_EVENT(TBaseDelegate<void>, OnLMBOver)
	SLATE_END_ARGS()

	void Construct(const FArguments& args);

	//overrides from SWidget
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

public:
	TBaseDelegate<void> _OnLMBOver;
};