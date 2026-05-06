#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonTextBlock.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"

UE_UI_DEFINE_WIDGET_LIFECYCLE(UGorgeousCommonTextBlock)

void UGorgeousCommonTextBlock::OnThemeApplied_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// Base handles universal reflection (Fonts, Sizes, etc.) — no super implementation for UCommonTextBlock

	// We only handle the special "interpolated" colors here
	for (auto& Pair : TargetThemeColors)
	{
		if (Pair.Key == "TextColor" || Pair.Key == "PrimaryColor")
		{
			FLinearColor DisplayColor = CurrentThemeColors.Contains(Pair.Key) ? CurrentThemeColors[Pair.Key] : Pair.Value;
			SetColorAndOpacity(FSlateColor(DisplayColor));
		}
	}
}

void UGorgeousCommonTextBlock::NativeConstruct()
{
	UE_UI_REGISTER_WIDGET()
}

void UGorgeousCommonTextBlock::NativeDestruct()
{
	UE_UI_UNREGISTER_WIDGET()
}

void UGorgeousCommonTextBlock::OnThemeApplied_BP_Implementation(const UGorgeousUITheme_DA* Theme)
{
    // Default: no-op. Override in Blueprints to react to theme changes.
}

UE_UI_IMPLEMENT_THEME_BRIDGE(UGorgeousCommonTextBlock)
