#include "LTerrainEditor.h"
#include "LSymbolSelector.h"
#include "LSymbolBox.h"

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

void SLSymbolSelector::Construct(const FArguments & args)
{
	lTerrainModule = FLTerrainEditorModule::GetModule();

	Reconstruct();
}

void SLSymbolSelector::Reconstruct()
{
	TSharedRef<SVerticalBox> vertBox = SNew(SVerticalBox);
	for (int brushi = 0, i = 0; i < 20; ++i)
	{
		TSharedRef<SHorizontalBox> horBox = SNew(SHorizontalBox);
		for (int j = 0; j < 3; ++j, ++brushi) //Drop-down is 3 brushes selections wide
		{
			//if we're done, fill rest of current row with empty slots then break outer loop
			if (brushi >= lTerrainModule->lSystem.symbols.Num())
			{
				horBox->AddSlot()
				[
					//spawns dummy box
					SNew(SLSymbolBox)
					.Symbol(nullptr)
				];
			}
			else
			{
				horBox->AddSlot()
				[
					SNew(SButton)
					.OnClicked_Lambda([this, brushi]() {
						this->selectedSymbol = this->lTerrainModule->lSystem.symbols[brushi];
						this->Reconstruct();
						return FReply::Handled();
					})
					[
						SNew(SLSymbolBox)
						.Symbol(lTerrainModule->lSystem.symbols[brushi])
					]
				];
			}
		}
		vertBox->AddSlot()
		[
			horBox
		];

		if (brushi >= lTerrainModule->lSystem.symbols.Num()) break;
	}

	ChildSlot
	[
		SNew(SComboButton)
		.ButtonContent()
		[
			SNew(SLSymbolBox)
			.Symbol(selectedSymbol)
		]
		.MenuContent()
		[
			vertBox
		]
	];
}

#undef LOCTEXT_NAMESPACE