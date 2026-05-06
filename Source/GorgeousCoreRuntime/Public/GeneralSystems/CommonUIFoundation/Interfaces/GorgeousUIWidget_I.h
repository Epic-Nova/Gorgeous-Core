// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
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

	/** Returns the widget/object instance for this UI element. */
	virtual UObject* GetAsWidget() = 0;

	/** Applies a theme to this widget. Called by the subsystem on theme changes. */
	virtual void OnThemeApplied(const UGorgeousUITheme_DA* Theme) = 0;
};
