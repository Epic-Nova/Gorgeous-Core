// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonTextBlock.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperImplementation.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "Styling/SlateColor.h"

UE_UI_IMPLEMENT_WIDGET_INTERFACE(UGorgeousCommonTextBlock)

void UGorgeousCommonTextBlock::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	UE_UI_REGISTER_WIDGET_RAW()
}

void UGorgeousCommonTextBlock::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();
	UE_UI_REGISTER_WIDGET_RAW()
}

void UGorgeousCommonTextBlock::ApplyThemeInterpolation(const UGorgeousUITheme_DA* Theme)
{
	if (Theme && TypographyTag.IsValid())
	{
		FGorgeousUITypography_S TypeInfo = Theme->GetTypography(TypographyTag);
		SetFont(TypeInfo.Font);
		SetColorAndOpacity(FSlateColor(TypeInfo.Color));
	}
}
