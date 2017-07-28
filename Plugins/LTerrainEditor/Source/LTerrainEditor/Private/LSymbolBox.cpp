#include "LTerrainEditor.h"
#include "LSymbolBox.h"

void SLSymbolBox::Construct(const FArguments & args)
{
	if (!args._Symbol.IsValid()) //spawn dummy box
	{
		ChildSlot
		[
			SNew(SBox)
			.MinDesiredHeight(32)
			.MinDesiredWidth(32)
		];
	}
	else if (args._Symbol->texture.IsValid()) //spawn image box
	{
		ChildSlot
		[
			SNew(SBox)
			.MinDesiredHeight(32)
			.MinDesiredWidth(32)
			[
				SNew(SImage)
				.Image_Lambda([args]()->const FSlateBrush* {
					FSlateBrush* newbrush = new FSlateBrush();
					newbrush->SetResourceObject(args._Symbol->texture.GetAsset());
					return newbrush;
				})
			]
		];
	}
	else //spawn text box
	{
		ChildSlot
		[
			SNew(SBox)
			.MinDesiredHeight(32)
			.MinDesiredWidth(32)
			[
				SNew(SBox)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Text_Lambda([args]()->FText {
						return FText::FromString(FString::Chr(args._Symbol->symbol));
					})
				]
			]
		];
	}
}
