#pragma once
#include "LTerrainEditor.h"

class SLSymbolBox : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLSymbolBox) {}
	SLATE_ATTRIBUTE(LSymbolPtr, Symbol)
	SLATE_END_ARGS()

	void Construct(const FArguments& args);
};