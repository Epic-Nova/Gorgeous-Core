// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousActivatableWidget.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GorgeousActionBar_CAW.generated.h"

/**
 * Base widget for the HUD Action Bar stripe.
 * Displays available input actions, their icons, and names.
 */
UCLASS(Abstract, Blueprintable)
class GORGEOUSCORERUNTIME_API UGorgeousActionBar_CAW : public UGorgeousActivatableWidget
{
	GENERATED_BODY()

public:
	UGorgeousActionBar_CAW(const FObjectInitializer& ObjectInitializer);

public:
	/** 
	 * Updates the action bar with new entries. 
	 * Blueprint should implement this to rebuild the UI (e.g. clear box and add children).
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous|UI")
	void UpdateActionBar(const TArray<FGorgeousActionBarEntry_S>& Entries);

protected:
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
};
