// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "DetailExtensions/GorgeousDetailExtension.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "GorgeousTestDetailExtension.generated.h"

/**
 * A test detail extension that adds a "Gorgeous!" button to a property row.
 */
UCLASS()
class UGorgeousTestDetailExtension : public UGorgeousDetailExtension
{
	GENERATED_BODY()

public:
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow) override
	{
		HeaderRow.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				PropertyHandle->CreatePropertyValueWidget()
			]
			+ SHorizontalBox::Slot()
			.Padding(5, 0)
			.AutoWidth()
			[
				SNew(SButton)
				.OnClicked_Lambda([]() {
					GT_S_LOG("GT.Editor.Test", TEXT("Gorgeous! Button Clicked!"));
					return FReply::Handled();
				})
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("✨ Gorgeous!")))
				]
			]
		];
	}

	virtual FName GetExtensionName() const override { return FName("Test"); }
};
