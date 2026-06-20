// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Styling/SlateBrush.h"
#include "Fonts/SlateFontInfo.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
#include "InputTriggers.h"
#include "Types/SlateEnums.h"
#include "GorgeousUIFoundationStructures.generated.h"

/**
 * A generic container for UI update values.
 * This is used to pass data through the Signal Bridge.
 */
USTRUCT(BlueprintType)
struct FGorgeousUIUpdatePayload
{
	GENERATED_BODY()

	/**
	 * Map of property names (e.g., "Text", "Color", "Progress") to their new values.
	 * Using FInstancedStruct allows us to pass any data type (float, FText, FLinearColor, etc.).
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	TMap<FName, FInstancedStruct> Updates;

	/**
	 * Optional animation tag to play when this update is applied.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag AnimationTag;

	/**
	 * Optional ID used to route this update to a specific widget instance 
	 * among many sharing the same BindingTag (e.g. Slot Index).
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FName RoutingID;

	/**
	 * Optional ID used to distinguish between multiple containers (e.g. Inventory ID).
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGuid InventoryID;
};

/**
 * Base struct for specific widget update payloads.
 * These are used by processor objects to cast the instanced struct data.
 */
USTRUCT(BlueprintType)
struct FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()
};

/** Payload for text-based updates. */
USTRUCT(BlueprintType)
struct FGorgeousTextUpdatePayload : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FText Text;

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FLinearColor Color = FLinearColor::White;
};

/** Payload for progress-based updates. */
USTRUCT(BlueprintType)
struct FGorgeousProgressUpdatePayload : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	float Progress = 0.0f;
};

/** Payload for panel-based updates (child injection). */
USTRUCT(BlueprintType)
struct FGorgeousPanelUpdatePayload : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

	/** Optional child widget to add. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	UUserWidget* ChildWidget = nullptr;

	/** Whether to clear the children before adding. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	bool bClearChildren = false;

	/** Optional child widget to remove. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	UUserWidget* WidgetToRemove = nullptr;
};

/** Payload for focus requests. */
USTRUCT(BlueprintType)
struct FGorgeousFocusRequestPayload : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

	/** The binding tag of the widget that should receive focus. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag TargetTag;

	/** Optional routing ID to request focus on a specific widget instance. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FName RoutingID;
};

/** Payload broadcast when focus changes. */
USTRUCT(BlueprintType)
struct FGorgeousFocusChangedPayload_S : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

	/** The binding tag of the widget that received focus. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag FocusedTag;

	/** The routing ID of the widget that received focus. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FName RoutingID;
};

/** Enum defining fallback strategies when focus restoration fails. */
UENUM(BlueprintType)
enum class EGorgeousFocusFallbackStrategy_E : uint8
{
	None = 0 UMETA(DisplayName = "Let CommonUI Decide"),
	FocusSpecificWidget = 1 UMETA(DisplayName = "Focus Specific Widget")
};

/** Configuration for fallback focus. */
USTRUCT(BlueprintType)
struct FGorgeousFocusFallbackConfig_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	EGorgeousFocusFallbackStrategy_E Strategy = EGorgeousFocusFallbackStrategy_E::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation", meta = (EditCondition = "Strategy == EGorgeousFocusFallbackStrategy_E::FocusSpecificWidget", EditConditionHides))
	FGameplayTag TargetTag;
};

/** Data structural mapping for directional focus routing. */
USTRUCT(BlueprintType)
struct FGorgeousFocusRoute_S
{
	GENERATED_BODY()

	/** The starting binding tag. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag SourceTag;

	/** The navigation direction that triggers this route. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	EUINavigation Direction = EUINavigation::Invalid;

	/** The target binding tag to focus. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag TargetTag;
};

/** Internal structure for deferred focus requests. */
USTRUCT(BlueprintType)
struct FGorgeousPendingFocusRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag TargetTag;

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FName RoutingID;

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	double ExpirationTime = 0.0;
};

/** Payload for grid panel updates. */
USTRUCT(BlueprintType)
struct FGorgeousGridUpdatePayload : public FGorgeousPanelUpdatePayload
{
	GENERATED_BODY()

	/** Row index for the operation. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	int32 Row = 0;

	/** Column index for the operation. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	int32 Column = 0;
};

/**
 * Payload for triggering an input action via signal.
 */
USTRUCT(BlueprintType)
struct FGorgeousInputActionPayload : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

	/** The input action tag to trigger (e.g., UI.Action.Back). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag ActionTag;

	/** If valid, only routes to this specific widget. Otherwise routes to the global active stack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag TargetWidgetTag;
};

/**
 * A group of icons for a specific action, mapped by platform name.
 */
USTRUCT(BlueprintType)
struct FGorgeousPlatformIconGroup_S
{
	GENERATED_BODY()

	/** 
	 * Map of Platform Name to Icon Brush.
	 * Common Keys: Keyboard, Xbox, PlayStation, Switch, Mobile
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation", meta = (GetOptions = "GetPlatformOptions"))
	TMap<FName, FSlateBrush> PlatformIcons;
};

/** Typography style defining font and color. */
USTRUCT(BlueprintType)
struct FGorgeousUITypography_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography")
	FSlateFontInfo Font;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography")
	FLinearColor Color = FLinearColor::White;
};

/** Payload for registering a UI layer stack. */
USTRUCT(BlueprintType)
struct FGorgeousRegisterLayerPayload : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag LayerTag;

	/** The stack widget being registered. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	TObjectPtr<UObject> LayerWidget = nullptr;
};

/** Configuration for an Input Mapping Context to be pushed by a UI state. */
USTRUCT(BlueprintType)
struct FGorgeousInputMappingConfig_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	TSoftObjectPtr<class UInputMappingContext> InputMapping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous Core|Common UI Foundation")
	bool bIsDefaultMapping = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation", meta = (EditCondition = "!bIsDefaultMapping", EditConditionHides))
	int32 Priority = 0;
};

/** Metadata for a Gorgeous input binding. */
USTRUCT(BlueprintType)
struct FGorgeousInputBindingInfo_S
{
	GENERATED_BODY()

	/** The Enhanced Input Action to bind to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<class UInputAction> Action;

	/** If true, this action will be displayed in the HUD's action bar. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bShouldDisplayInActionBar = true;
	
	/** If true, this input will be consumed once handled. Turn off to allow lower-priority systems or actions to see the key. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bConsumeInput = false;

	/** Friendly name for the action. If empty, uses the tag name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FText DisplayName;

	/** Trigger events to bind for this action. If empty, defaults to Started, Triggered, Completed, Canceled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TArray<ETriggerEvent> TriggerEventsToBind;

	/** Specify which layers this action can be displayed in. If empty, it's allowed on any layer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (Categories = "GT.UI.Layer", EditCondition = "bShouldDisplayInActionBar", EditConditionHides))
	TArray<FGameplayTag> AllowedActionBarLayers;
};

/** Data for a single entry in the HUD action bar. */
USTRUCT(BlueprintType)
struct FGorgeousActionBarEntry_S
{
	GENERATED_BODY()

	/** The resolved icon brush for the action. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Bar")
	FSlateBrush Icon;

	/** The display name of the action. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Bar")
	FText ActionName;
};

class UGorgeousUIMessageConfig_DA;

/**
 * Data passed when requesting a new Gorgeous Message/Dialog.
 */
USTRUCT(BlueprintType)
struct FGorgeousUIMessageRequest
{
	GENERATED_BODY()

	/** Unique ID for this specific message instance to track the result. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGuid RequestID;

	/** The title text for the dialog. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FText Title;

	/** The main body/message text. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FText Message;

	/** The configuration asset defining the look and feel. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	TObjectPtr<const UGorgeousUIMessageConfig_DA> Config;

	/** Optional custom data to pass to the message widget. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	TMap<FName, FString> Metadata;

	FGorgeousUIMessageRequest()
		: RequestID(FGuid::NewGuid()), Config(nullptr)
	{}
};

/**
 * Data passed back when a message is resolved (button clicked).
 */
USTRUCT(BlueprintType)
struct FGorgeousUIMessageResult
{
	GENERATED_BODY()

	/** The ID of the request this result belongs to. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGuid RequestID;

	/** The tag of the button that was clicked (e.g., "Confirm", "Cancel", "Repair"). */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FName ResultTag;

	FGorgeousUIMessageResult()
		: ResultTag(NAME_None)
	{}
};
