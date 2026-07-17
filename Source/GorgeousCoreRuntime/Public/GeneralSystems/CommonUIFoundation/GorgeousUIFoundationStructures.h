// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Styling/SlateBrush.h"
#include "Fonts/SlateFontInfo.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include "Types/SlateEnums.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
#include "InputTriggers.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousUIFoundationStructures.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Contains UI update values routed through the Signal Bridge.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousUIUpdatePayload
{
	GENERATED_BODY()

	// Maps widget property names to their replacement values.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	TMap<FName, FInstancedStruct> Updates;

	// Optional animation tag to play when this update is applied.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag AnimationTag;

	// Routes the update to a specific widget instance when set.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FName RoutingID;

	// Optional ID used to distinguish between multiple containers (e.g. Inventory ID).
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGuid InventoryID;
};

/**
 * Serves as the common base for typed widget update payloads.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()
};

/**
 * Carries text and color values for a widget update.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousTextUpdatePayload : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

	// Supplies the replacement text.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FText Text;

	// Supplies the replacement text color.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FLinearColor Color = FLinearColor::White;
};

/**
 * Carries a progress value for a widget update.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousProgressUpdatePayload : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

	// Supplies the replacement progress value.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	float Progress = 0.0f;
};

/**
 * Carries child-widget changes for a panel update.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousPanelUpdatePayload : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

	// Optional child widget to add.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	UUserWidget* ChildWidget = nullptr;

	// Whether to clear the children before adding.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	bool bClearChildren = false;

	// Optional child widget to remove.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	UUserWidget* WidgetToRemove = nullptr;
};

/**
 * Carries the target of a focus request.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousFocusRequestPayload : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

	// The binding tag of the widget that should receive focus.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag TargetTag;

	// Optional routing ID to request focus on a specific widget instance.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FName RoutingID;
};

/**
 * Reports the widget selected after focus changes.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousFocusChangedPayload_S : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

	// The binding tag of the widget that received focus.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag FocusedTag;

	// The routing ID of the widget that received focus.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FName RoutingID;
};

/**
 * Enumerates fallback strategies when focus restoration fails.
 *
 * @author Nils Bergemann
 */
UENUM(BlueprintType, meta = (ShortTooltip = "Fallback strategies used when focus restoration fails."))
enum class EGorgeousFocusFallbackStrategy_E : uint8
{
	None = 0 UMETA(DisplayName = "Let CommonUI Decide", ToolTip = "Defers fallback focus selection to CommonUI."),
	FocusSpecificWidget = 1 UMETA(DisplayName = "Focus Specific Widget", ToolTip = "Focuses the configured widget tag." )
};

/**
 * Configures a fallback target for failed focus restoration.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousFocusFallbackConfig_S
{
	GENERATED_BODY()

	// Selects the fallback behavior.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	EGorgeousFocusFallbackStrategy_E Strategy = EGorgeousFocusFallbackStrategy_E::None;

	// Identifies the widget to focus for the explicit strategy.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation", meta = (EditCondition = "Strategy == EGorgeousFocusFallbackStrategy_E::FocusSpecificWidget", EditConditionHides))
	FGameplayTag TargetTag;
};

/**
 * Maps a navigation direction between two widget tags.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousFocusRoute_S
{
	GENERATED_BODY()

	// The starting binding tag.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag SourceTag;

	// The navigation direction that triggers this route.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	EUINavigation Direction = EUINavigation::Invalid;

	// The target binding tag to focus.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag TargetTag;
};

/**
 * Stores a focus request until it can be applied.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousPendingFocusRequest
{
	GENERATED_BODY()

	// Identifies the target widget.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag TargetTag;

	// Identifies the target widget instance.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FName RoutingID;

	// Stores the request expiration time.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	double ExpirationTime = 0.0;
};

/**
 * Carries row and column values for a grid-panel update.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousGridUpdatePayload : public FGorgeousPanelUpdatePayload
{
	GENERATED_BODY()

	// Row index for the operation.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	int32 Row = 0;

	// Column index for the operation.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	int32 Column = 0;
};

/**
 * Carries an input action request received through a signal.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousInputActionPayload : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

	// The input action tag to trigger (e.g., UI.Action.Back).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag ActionTag;

	// If valid, only routes to this specific widget. Otherwise routes to the global active stack.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag TargetWidgetTag;
};

/**
 * Maps platform names to icons for an action.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousPlatformIconGroup_S
{
	GENERATED_BODY()

	// Maps each supported platform name to its icon brush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation", meta = (GetOptions = "GetPlatformOptions"))
	TMap<FName, FSlateBrush> PlatformIcons;
};

/**
 * Defines a font and color typography style.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousUITypography_S
{
	GENERATED_BODY()

	// Supplies the font used by the style.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography")
	FSlateFontInfo Font;

	// Supplies the text color used by the style.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography")
	FLinearColor Color = FLinearColor::White;
};

/**
 * Carries the data required to register a UI layer stack.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousRegisterLayerPayload : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

	// Identifies the registered layer.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag LayerTag;

	// The stack widget being registered.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	TObjectPtr<UObject> LayerWidget = nullptr;
};

/**
 * Configures an input mapping context pushed by a UI state.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousInputMappingConfig_S
{
	GENERATED_BODY()

	// References the input mapping context to push.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	TSoftObjectPtr<class UInputMappingContext> InputMapping;

	// Indicates whether this mapping is the default mapping.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous Core|Common UI Foundation")
	bool bIsDefaultMapping = false;

	// Determines this mapping's priority when it is not the default.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation", meta = (EditCondition = "!bIsDefaultMapping", EditConditionHides))
	int32 Priority = 0;
};

/**
 * Defines metadata for a Gorgeous input binding.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousInputBindingInfo_S
{
	GENERATED_BODY()

	// The Enhanced Input Action to bind to.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<class UInputAction> Action;

	// If true, this action will be displayed in the HUD's action bar.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bShouldDisplayInActionBar = true;

	// If true, this input will be consumed once handled. Turn off to allow lower-priority systems or actions to see the key.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bConsumeInput = false;

	// Friendly name for the action. If empty, uses the tag name.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FText DisplayName;

	// Trigger events to bind for this action. If empty, defaults to Started, Triggered, Completed, Canceled.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TArray<ETriggerEvent> TriggerEventsToBind;

	// Specify which layers this action can be displayed in. If empty, it's allowed on any layer.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (Categories = "GT.UI.Layer", EditCondition = "bShouldDisplayInActionBar", EditConditionHides))
	TArray<FGameplayTag> AllowedActionBarLayers;
};

/**
 * Stores the resolved data for one HUD action-bar entry.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousActionBarEntry_S
{
	GENERATED_BODY()

	// The resolved icon brush for the action.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Bar")
	FSlateBrush Icon;

	// The display name of the action.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Bar")
	FText ActionName;
};

class UGorgeousUIMessageConfig_DA;

/**
 * Carries the data required to request a message dialog.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousUIMessageRequest
{
	GENERATED_BODY()

	// Unique ID for this specific message instance to track the result.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGuid RequestID;

	// The title text for the dialog.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FText Title;

	// The main body/message text.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FText Message;

	// The configuration asset defining the look and feel.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	TObjectPtr<const UGorgeousUIMessageConfig_DA> Config;

	// Optional custom data to pass to the message widget.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	TMap<FName, FString> Metadata;

	FGorgeousUIMessageRequest()
		: RequestID(FGuid::NewGuid()), Config(nullptr)
	{}
};

/**
 * Carries the result returned when a message dialog is resolved.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousUIMessageResult
{
	GENERATED_BODY()

	// The ID of the request this result belongs to.
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGuid RequestID;

	// The tag of the button that was clicked (e.g., "Confirm", "Cancel", "Repair").
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FName ResultTag;

	FGorgeousUIMessageResult()
		: ResultTag(NAME_None)
	{}
};