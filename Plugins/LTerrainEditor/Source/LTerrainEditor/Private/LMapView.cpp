#include "LTerrainEditor.h"
#include "LMapView.h"
#include "LSymbolBox.h"

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

void SLMapView::Construct(const FArguments & args)
{
	Reconstruct(args._Map);
}

void SLMapView::Reconstruct(LSymbol2DMapPtr item)
{
	if (!item.IsValid()) return;

	TSharedRef<SUniformGridPanel> ruleGridPanel = SNew(SUniformGridPanel)
		.SlotPadding(2);

	for (int i = 0; i < item->Num(); ++i)
	{
		for (int j = 0; j < (*item)[i].Num(); ++j)
		{
			ruleGridPanel->AddSlot(j, i)
			[
				SNew(SLSymbolBox)
				.Symbol_Lambda([item, i, j]() {
					return (*item)[i][j];
				})
			];
		}
	}

	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.Padding(2)
				[
					ruleGridPanel
				]
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE