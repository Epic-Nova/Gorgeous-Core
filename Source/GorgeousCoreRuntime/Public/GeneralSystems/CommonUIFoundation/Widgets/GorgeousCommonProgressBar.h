// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ProgressBar.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperMacros.h"
#include "GorgeousCommonProgressBar.generated.h"

/**
 * Overridden ProgressBar with Signal Bridge support.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousCommonProgressBar : public UProgressBar, public IGorgeousUIWidget_I
{
	GENERATED_BODY()

public:
	UE_UI_WIDGET_INTERFACE_BOILERPLATE()

	UGorgeousCommonProgressBar(const FObjectInitializer& ObjectInitializer);

	/** Binding Tag for Signal Bridge routing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
	FGameplayTag BindingTag;

	/** Enables the per-widget style allow list. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI|Style")
	bool bUseStylePropertyAllowList = true;

	/** Properties that can be styled by themes or Signal Bridge payloads. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI|Style")
	TSet<FName> StylePropertyAllowList;

	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous UI", meta = (DisplayName = "On Theme Applied"))
	void OnThemeApplied_BP(const UGorgeousUITheme_DA* Theme);

protected:
	virtual void SynchronizeProperties() override;
	virtual void OnWidgetRebuilt() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
};
