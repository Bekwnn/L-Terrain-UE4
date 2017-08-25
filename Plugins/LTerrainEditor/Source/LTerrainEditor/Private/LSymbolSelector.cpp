#include "LTerrainEditor.h"
#include "LSymbolSelector.h"
#include "LSymbolBox.h"

#define LOCTEXT_NAMESPACE "FLTerrainEditorModule"

void SLSymbolSelector::Construct(const FArguments & args)
{
	lTerrainModule = FLTerrainEditorModule::GetModule();
	selectedSymbol = args._StartSymbol.Get();
	_OnSelectionClose = args._OnSelectionClose;

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
						FSlateApplication::Get().DismissAllMenus();
						_OnSelectionClose.ExecuteIfBound(this->selectedSymbol);
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
			.Symbol_Lambda([this]()->LSymbolPtr {
				return this->selectedSymbol;
			})
		]
		.MenuContent()
		[
			vertBox
		]
		.OnMenuOpenChanged_Lambda([this](bool bIsOpen) {
			if (!bIsOpen) this->Reconstruct();
		})
	];
}

#undef LOCTEXT_NAMESPACE