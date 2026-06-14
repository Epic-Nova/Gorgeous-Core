// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Styling/SlateBrush.h"
#include "Fonts/SlateFontInfo.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
<<<<<<< HEAD
#include "InputTriggers.h"
=======
>>>>>>> b4c134c (Some other changes i dont remember)
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
<<<<<<< HEAD
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
=======
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
>>>>>>> b4c134c (Some other changes i dont remember)
	TMap<FName, FInstancedStruct> Updates;

	/**
	 * Optional animation tag to play when this update is applied.
	 */
<<<<<<< HEAD
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
=======
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
>>>>>>> b4c134c (Some other changes i dont remember)
	FGameplayTag AnimationTag;

	/**
	 * Optional ID used to route this update to a specific widget instance 
	 * among many sharing the same BindingTag (e.g. Slot Index).
	 */
<<<<<<< HEAD
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
=======
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
>>>>>>> b4c134c (Some other changes i dont remember)
	FName RoutingID;

	/**
	 * Optional ID used to distinguish between multiple containers (e.g. Inventory ID).
	 */
<<<<<<< HEAD
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
=======
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
>>>>>>> b4c134c (Some other changes i dont remember)
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

<<<<<<< HEAD
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FText Text;

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
=======
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	FText Text;

	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
>>>>>>> b4c134c (Some other changes i dont remember)
	FLinearColor Color = FLinearColor::White;
};

/** Payload for progress-based updates. */
USTRUCT(BlueprintType)
struct FGorgeousProgressUpdatePayload : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

<<<<<<< HEAD
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
=======
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
>>>>>>> b4c134c (Some other changes i dont remember)
	float Progress = 0.0f;
};

/** Payload for panel-based updates (child injection). */
USTRUCT(BlueprintType)
struct FGorgeousPanelUpdatePayload : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

	/** Optional child widget to add. */
<<<<<<< HEAD
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	UUserWidget* ChildWidget = nullptr;

	/** Whether to clear the children before adding. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	bool bClearChildren = false;

	/** Optional child widget to remove. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
=======
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	UUserWidget* ChildWidget = nullptr;

	/** Whether to clear the children before adding. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	bool bClearChildren = false;

	/** Optional child widget to remove. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
>>>>>>> b4c134c (Some other changes i dont remember)
	UUserWidget* WidgetToRemove = nullptr;
};

/** Payload for focus requests. */
USTRUCT(BlueprintType)
struct FGorgeousFocusRequestPayload : public FGorgeousUIBaseUpdatePayload
{
	GENERATED_BODY()

	/** The binding tag of the widget that should receive focus. */
<<<<<<< HEAD
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
=======
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
>>>>>>> b4c134c (Some other changes i dont remember)
	FGameplayTag TargetTag;
};

/** Payload for grid panel updates. */
USTRUCT(BlueprintType)
struct FGorgeousGridUpdatePayload : public FGorgeousPanelUpdatePayload
{
	GENERATED_BODY()

	/** Row index for the operation. */
<<<<<<< HEAD
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	int32 Row = 0;

	/** Column index for the operation. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
=======
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	int32 Row = 0;

	/** Column index for the operation. */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
>>>>>>> b4c134c (Some other changes i dont remember)
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
<<<<<<< HEAD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag ActionTag;

	/** If valid, only routes to this specific widget. Otherwise routes to the global active stack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
=======
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	FGameplayTag ActionTag;

	/** If valid, only routes to this specific widget. Otherwise routes to the global active stack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
>>>>>>> b4c134c (Some other changes i dont remember)
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
<<<<<<< HEAD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation", meta = (GetOptions = "GetPlatformOptions"))
=======
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI", meta = (GetOptions = "GetPlatformOptions"))
>>>>>>> b4c134c (Some other changes i dont remember)
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

<<<<<<< HEAD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGameplayTag LayerTag;

	/** The stack widget being registered. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
=======
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	FGameplayTag LayerTag;

	/** The stack widget being registered. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
>>>>>>> b4c134c (Some other changes i dont remember)
	TObjectPtr<UObject> LayerWidget = nullptr;
};

/** Configuration for an Input Mapping Context to be pushed by a UI state. */
USTRUCT(BlueprintType)
struct FGorgeousInputMappingConfig_S
{
	GENERATED_BODY()

<<<<<<< HEAD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	TSoftObjectPtr<class UInputMappingContext> InputMapping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous Core|Common UI Foundation")
	bool bIsDefaultMapping = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation", meta = (EditCondition = "!bIsDefaultMapping", EditConditionHides))
=======
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
	TSoftObjectPtr<class UInputMappingContext> InputMapping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous UI")
>>>>>>> b4c134c (Some other changes i dont remember)
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
<<<<<<< HEAD
	
	/** If true, this input will be consumed once handled. Turn off to allow lower-priority systems or actions to see the key. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bConsumeInput = false;
=======
>>>>>>> b4c134c (Some other changes i dont remember)

	/** Friendly name for the action. If empty, uses the tag name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FText DisplayName;
<<<<<<< HEAD

	/** Trigger events to bind for this action. If empty, defaults to Started, Triggered, Completed, Canceled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TArray<ETriggerEvent> TriggerEventsToBind;

	/** Specify which layers this action can be displayed in. If empty, it's allowed on any layer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (Categories = "GT.UI.Layer", EditCondition = "bShouldDisplayInActionBar", EditConditionHides))
	TArray<FGameplayTag> AllowedActionBarLayers;
=======
>>>>>>> b4c134c (Some other changes i dont remember)
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
<<<<<<< HEAD
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
=======
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
>>>>>>> b4c134c (Some other changes i dont remember)
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
<<<<<<< HEAD
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
	FGuid RequestID;

	/** The tag of the button that was clicked (e.g., "Confirm", "Cancel", "Repair"). */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous Core|Common UI Foundation")
=======
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
	FGuid RequestID;

	/** The tag of the button that was clicked (e.g., "Confirm", "Cancel", "Repair"). */
	UPROPERTY(BlueprintReadWrite, Category = "Gorgeous UI")
>>>>>>> b4c134c (Some other changes i dont remember)
	FName ResultTag;

	FGorgeousUIMessageResult()
		: ResultTag(NAME_None)
	{}
};
