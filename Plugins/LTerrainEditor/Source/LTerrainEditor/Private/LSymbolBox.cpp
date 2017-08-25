#include "LTerrainEditor.h"
#include "LSymbolBox.h"

#define SYMBOL_BOX_SIZE 32

void SLSymbolBox::Construct(const FArguments & args)
{
	_OnLMBOver = args._OnLMBOver;

	if (!args._Symbol.Get().IsValid()) //spawn dummy box
	{
		SetToolTipText(FText::FromString("Null Symbol"));

		ChildSlot
		[
			SNew(SBox)
			.MinDesiredHeight(SYMBOL_BOX_SIZE)
			.MinDesiredWidth(SYMBOL_BOX_SIZE)
		];
	}
	else if (args._Symbol.Get()->texture.IsValid()) //spawn image box
	{
		SetToolTipText(FText::FromString(args._Symbol.Get()->name));

		ChildSlot
		[
			SNew(SBox)
			.MinDesiredHeight(SYMBOL_BOX_SIZE)
			.MinDesiredWidth(SYMBOL_BOX_SIZE)
			[
				SNew(SImage)
				.Image_Lambda([args]()->const FSlateBrush* {
					FSlateBrush* newbrush = new FSlateBrush();
					newbrush->SetResourceObject(args._Symbol.Get()->texture.GetAsset());
					return newbrush;
				})
			]
		];
	}
	else //spawn text box
	{
		SetToolTipText(FText::FromString(args._Symbol.Get()->name));

		ChildSlot
		[
			SNew(SBox)
			.MinDesiredHeight(SYMBOL_BOX_SIZE)
			.MinDesiredWidth(SYMBOL_BOX_SIZE)
			[
				SNew(SBox)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.Text_Lambda([args]()->FText {
						return FText::FromString(FString::Chr(args._Symbol.Get()->symbol));
					})
				]
			]
		];
	}
}

FReply SLSymbolBox::OnMouseButtonDown(const FGeometry & MyGeometry, const FPointerEvent & MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && _OnLMBOver.IsBound())
	{
		_OnLMBOver.Execute();
		return FReply::Handled();
	}
	else return FReply::Unhandled();
}

void SLSymbolBox::OnMouseEnter(const FGeometry & MyGeometry, const FPointerEvent & MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && _OnLMBOver.IsBound())
	{
		_OnLMBOver.Execute();
	}
}

#undef SYMBOL_BOX_SIZE