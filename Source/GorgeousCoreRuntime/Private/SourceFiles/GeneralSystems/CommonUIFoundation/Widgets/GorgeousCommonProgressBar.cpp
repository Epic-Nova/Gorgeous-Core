#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonProgressBar.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"

UE_UI_DEFINE_WIDGET_LIFECYCLE(UGorgeousCommonProgressBar)

void UGorgeousCommonProgressBar::OnThemeApplied_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// No base implementation on UProgressBar

	FName TargetKey = "";
	if (TargetThemeColors.Contains("ProgressColor")) TargetKey = "ProgressColor";
	else if (TargetThemeColors.Contains("AccentColor")) TargetKey = "AccentColor";

	if (TargetKey != "")
	{
		FLinearColor DisplayColor = CurrentThemeColors.Contains(TargetKey) ? CurrentThemeColors[TargetKey] : TargetThemeColors[TargetKey];
		SetFillColorAndOpacity(DisplayColor);
	}
}

void UGorgeousCommonProgressBar::NativeConstruct()
{
	UE_UI_REGISTER_WIDGET()
}

void UGorgeousCommonProgressBar::NativeDestruct()
{
	UE_UI_UNREGISTER_WIDGET()
}

void UGorgeousCommonProgressBar::OnThemeApplied_BP_Implementation(const UGorgeousUITheme_DA* Theme)
{
    // Default: no-op. Override in Blueprints to react to theme changes.
}

UE_UI_IMPLEMENT_THEME_BRIDGE(UGorgeousCommonProgressBar)
