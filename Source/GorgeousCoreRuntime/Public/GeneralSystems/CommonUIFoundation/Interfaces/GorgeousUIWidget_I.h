// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIOverlayConfig_DA.h"
#include "GorgeousUIWidget_I.generated.h"

class UGorgeousUITheme_DA;

UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousUIWidget_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for all widgets that participate in the Gorgeous UI Foundation system.
 */
class GORGEOUSCORERUNTIME_API IGorgeousUIWidget_I
{
	GENERATED_BODY()

public:
	/** Returns the binding tag for this widget. */
	virtual FGameplayTag GetBindingTag() const = 0;

	/** Returns the routing ID for this widget instance (e.g. Slot Index). */
	virtual FName GetRoutingID() const { return NAME_None; }

	/** Sets the routing ID for this widget instance. */
	virtual void SetRoutingID(FName InID) {}

	/** Returns true if the style allow list should be enforced. */
	virtual bool UseStylePropertyAllowList() const { return true; }

	/** Returns the style property allow list for this widget. */
	virtual const TSet<FName>& GetStylePropertyAllowList() const { static TSet<FName> Dummy; return Dummy; }

	/** Returns true if the property is allowed for styling updates. */
	virtual bool IsStylePropertyAllowed(FName PropertyName) const
	{
		return !UseStylePropertyAllowList() || GetStylePropertyAllowList().Contains(PropertyName);
	}

	/** Returns the widget/object instance for this UI element. */
	virtual UObject* GetAsWidget() = 0;

	/** Applies a theme to this widget. Called by the subsystem on theme changes. */
	virtual void OnThemeApplied(const UGorgeousUITheme_DA* Theme) {}

	/** Returns the widget's render opacity. Fallback for non-UWidget implementations. */
	virtual float GetWidgetOpacity() const { return 1.0f; }
	/** Sets the widget's render opacity. Fallback for non-UWidget implementations. */
	virtual void SetWidgetOpacity(float InOpacity) {}

	/** Returns the target opacity we are interping towards. */
	virtual float GetTargetOpacity() const { return 1.0f; }
	/** Returns the starting opacity of the current transition. */
	virtual float GetStartOpacity() const { return 1.0f; }
	/** Returns the interpolation config for opacity. */
	virtual struct FGorgeousUIInterpConfig_S GetOpacityInterpConfig() const { return {}; }

	/** Returns the current elapsed time of the opacity transition. */
	virtual float GetElapsedOpacityTime() const { return 0.0f; }
	/** Sets the current elapsed time of the opacity transition. */
	virtual void SetElapsedOpacityTime(float InTime) {}

	/** Returns true if we are currently interping opacity. */
	virtual bool IsInterpOpacity() const { return false; }
	/** Sets whether we are currently interping opacity. */
	virtual void SetIsInterpOpacity(bool bInInterp) {}

	/** Same for Theme interp... */
	virtual float GetElapsedThemeTime() const { return 0.0f; }
	virtual void SetElapsedThemeTime(float InTime) {}
	virtual bool IsInterpTheme() const { return false; }
	virtual void SetIsInterpTheme(bool bInInterp) {}
	virtual FGorgeousUIInterpConfig_S GetThemeInterpConfig() const { return {}; }
	virtual TMap<FName, FLinearColor>& GetStartThemeColors() { static TMap<FName, FLinearColor> Dummy; return Dummy; }
	virtual TMap<FName, FLinearColor>& GetTargetThemeColors() { static TMap<FName, FLinearColor> Dummy; return Dummy; }
	virtual TMap<FName, FLinearColor>& GetCurrentThemeColors() { static TMap<FName, FLinearColor> Dummy; return Dummy; }

	/** Notifies that the widget has finished its transition. */
	virtual void NotifyReadyForStateSwap() {}

	/** Processes interpolation for opacity and theme colors. */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous UI")
	void TickInterpolation(float DeltaTime);

	/** Applies a state-specific configuration. */
	virtual void ApplyOverlayConfig(const struct FGorgeousUIStateConfig& Config) = 0;
};
