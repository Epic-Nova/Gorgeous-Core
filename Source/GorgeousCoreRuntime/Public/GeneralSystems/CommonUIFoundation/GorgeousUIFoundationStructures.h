// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Styling/SlateBrush.h"
#include "Fonts/SlateFontInfo.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
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
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	TMap<FName, FInstancedStruct> Updates;

	/**
	 * Optional animation tag to play when this update is applied.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	FGameplayTag AnimationTag;

	/**
	 * Optional ID used to route this update to a specific widget instance 
	 * among many sharing the same BindingTag (e.g. Slot Index).
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	FName RoutingID;
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

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	FText Text;

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	FLinearColor Color = FLinearColor::White;
};

/** Payload for progress-based updates. */
USTRUCT(BlueprintType)
struct FGorgeousProgressUpdatePayload : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	float Progress = 0.0f;
};

/** Payload for panel-based updates (child injection). */
USTRUCT(BlueprintType)
struct FGorgeousPanelUpdatePayload : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

	/** Optional child widget to add. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	UUserWidget* ChildWidget = nullptr;

	/** Whether to clear the children before adding. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	bool bClearChildren = false;

	/** Optional child widget to remove. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	UUserWidget* WidgetToRemove = nullptr;
};

/** Payload for focus requests. */
USTRUCT(BlueprintType)
struct FGorgeousFocusRequestPayload : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

	/** The binding tag of the widget that should receive focus. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	FGameplayTag TargetTag;
};

/** Payload for grid panel updates. */
USTRUCT(BlueprintType)
struct FGorgeousGridUpdatePayload : public FGorgeousPanelUpdatePayload
{
	GENERATED_BODY()

	/** Row index for the operation. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	int32 Row = 0;

	/** Column index for the operation. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	FGameplayTag ActionTag;

	/** If valid, only routes to this specific widget. Otherwise routes to the global active stack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI", meta = (GetOptions = "GetPlatformOptions"))
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	FGameplayTag LayerTag;

	/** The stack widget being registered. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	TObjectPtr<UObject> LayerWidget = nullptr;
};

/** Configuration for an Input Mapping Context to be pushed by a UI state. */
USTRUCT(BlueprintType)
struct FGorgeousInputMappingConfig_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	TSoftObjectPtr<class UInputMappingContext> InputMapping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
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

	/** Friendly name for the action. If empty, uses the tag name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FText DisplayName;
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
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	FGuid RequestID;

	/** The title text for the dialog. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	FText Title;

	/** The main body/message text. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	FText Message;

	/** The configuration asset defining the look and feel. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	TObjectPtr<const UGorgeousUIMessageConfig_DA> Config;

	/** Optional custom data to pass to the message widget. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
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
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	FGuid RequestID;

	/** The tag of the button that was clicked (e.g., "Confirm", "Cancel", "Repair"). */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	FName ResultTag;

	FGorgeousUIMessageResult()
		: ResultTag(NAME_None)
	{}
};
