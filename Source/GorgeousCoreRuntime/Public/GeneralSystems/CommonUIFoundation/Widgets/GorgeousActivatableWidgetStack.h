// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "GameplayTagContainer.h"
#include "GorgeousActivatableWidgetStack.generated.h"

/**
 * A CommonActivatableWidgetStack that auto-registers itself as a layout layer.
 *
 * --- SETUP ---
 * 1. Place this widget in your UGorgeousPrimaryGameLayout Blueprint.
 * 2. Set the LayerTag property (e.g. UI.Layer.Game).
 * 3. Done. When the layout constructs, this stack registers itself automatically.
 *    No C++ required, no manual RegisterLayer calls.
 *
 * --- EXAMPLE LAYER HIERARCHY ---
 *   [Background] UGorgeousActivatableWidgetStack  (tag: UI.Layer.Background)
 *   [Game]       UGorgeousActivatableWidgetStack  (tag: UI.Layer.Game)
 *   [Modal]      UGorgeousActivatableWidgetStack  (tag: UI.Layer.Modal)
 *   [Loading]    UGorgeousActivatableWidgetStack  (tag: UI.Layer.Loading)
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Gorgeous Activatable Widget Stack"))
class GORGEOUSCORERUNTIME_API UGorgeousActivatableWidgetStack : public UCommonActivatableWidgetStack
{
	GENERATED_BODY()

public:
	/**
	 * The layer tag this stack registers as.
	 * Must be under the UI.Layer parent tag.
	 * Set this in the Blueprint defaults.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI", meta = (Categories = "UI.Layer"))
	FGameplayTag LayerTag;

protected:
	virtual void OnWidgetRebuilt() override;
};
