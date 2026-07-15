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

#include "CoreMinimal.h"
#include "GorgeousCoreMinimalShared.h"
#include "GeneralSystems/DebugAssist/DebugAssistStructures.h"
#include "InteractionFoundationStructures.generated.h"

USTRUCT(BlueprintType)
struct GORGEOUSCORERUNTIME_API FGorgeousInteractionSphereTraceParameters
{
    GENERATED_BODY()

    FGorgeousInteractionSphereTraceParameters()
        : Start(FVector::ZeroVector)
        , End(FVector::ZeroVector)
        , Radius(100.0f)
        , TraceChannel(ECC_WorldDynamic)
        , bTraceComplex(false)
        , bIgnoreSelf(true)
        , ActorsToIgnore()
        , DebugVisualParameters()
    {
        DebugVisualParameters.bEnabled = true;
        DebugVisualParameters.bDrawSweptSphere = true;
        DebugVisualParameters.bWireframe = false;
        DebugVisualParameters.TracePathColor = FColor::White;
        DebugVisualParameters.SweptSphereHitColor = FColor(0, 255, 0, 32);
        DebugVisualParameters.SweptSphereMissColor = FColor(255, 0, 0, 32);
        DebugVisualParameters.SweptSphereSteps = 10;
        DebugVisualParameters.SweptSphereSegments = 16;
        
        DebugVisualParameters.HitSphere.bDraw = false;
        
        DebugVisualParameters.HitBounds.bDraw = true;
        DebugVisualParameters.HitBounds.bDrawFilled = false;
        DebugVisualParameters.HitBounds.Inflation = 1.5f;
        DebugVisualParameters.HitBounds.bPulse = true;
    }

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
    FVector Start = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
    FVector End = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
    float Radius = 15.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_WorldDynamic;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
    bool bTraceComplex = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
    bool bIgnoreSelf = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
    TArray<AActor*> ActorsToIgnore;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
    FGorgeousDebugAssistVisualParameters DebugVisualParameters;
};

/**
 * Prompt payload returned by world items through the interaction foundation interface.
 * Represents a simple interaction widget.
 */
USTRUCT(BlueprintType, DisplayName = "Gorgeous Interaction Action Payload")
struct GORGEOUSCORERUNTIME_API FGorgeousInteractionActionPayload_S
{
    GENERATED_BODY()
    
    // Text to display when the action widget is visible
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FText InteractionPrompt;
    
    // Required hold time for this promt to fire
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float RequiredInteractionTime;
    
	// Optional Action UObject, commonly used in inventory systems to give meta context about what action needs to be performed on the inventory
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TSubclassOf<UObject> ActionClass;
	
    // Refresh flag to indicate bypass of the refresh system to forcefully redraw the promt widget
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bNeedsRefresh;
	
	FGorgeousInteractionActionPayload_S()
		: InteractionPrompt(FText::FromString("Interact"))
		, RequiredInteractionTime(0.0f)
		, ActionClass(nullptr)
		, bNeedsRefresh(false)
	{}
    
};

USTRUCT(BlueprintType, DisplayName = "Gorgeous Interaction Prompt Slot")
struct GORGEOUSCORERUNTIME_API FGorgeousInteractionPromptSlot_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FName SlotName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FInstancedStruct Payload;

	bool operator==(const FGorgeousInteractionPromptSlot_S& Other) const
	{
		return SlotName == Other.SlotName;
	}
};

FORCEINLINE uint32 GetTypeHash(const FGorgeousInteractionPromptSlot_S& Slot)
{
	return GetTypeHash(Slot.SlotName);
}

USTRUCT(BlueprintType, DisplayName = "Gorgeous Interaction Stat Entry")
struct GORGEOUSCORERUNTIME_API FGorgeousInteractionStatEntry_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FGameplayTag StatTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText DisplayLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText ValueText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bHideIfEmpty = true;

	bool operator==(const FGorgeousInteractionStatEntry_S& Other) const
	{
		return StatTag == Other.StatTag;
	}
};

FORCEINLINE uint32 GetTypeHash(const FGorgeousInteractionStatEntry_S& Entry)
{
	return GetTypeHash(Entry.StatTag);
}

/**
 * Prompt payload returned by world actors through the interaction foundation interface.
 * Represents a complex interaction widget with multiple slots and statistics.
 */
USTRUCT(BlueprintType, DisplayName = "Gorgeous Interaction Prompt Payload")
struct GORGEOUSCORERUNTIME_API FGorgeousInteractionPromptPayload_S
{
	GENERATED_BODY()

	// Displayed as the main header of the interaction prompt widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText PromptHeader;

	// Displayed as the secondary descriptor of the interaction prompt widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText PromptDescriptor;

	// Optional icon displayed in the interaction prompt widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TSoftObjectPtr<UTexture2D> PromptIcon;

	// Gameplay tags associated with the interaction, used for filtering and categorization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FGameplayTagContainer InteractionTags;

	// Interaction slots for the top section of the prompt widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TArray<FGorgeousInteractionPromptSlot_S> HeaderTopSlots;

	// Interaction slots for the main section of the prompt widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TArray<FGorgeousInteractionPromptSlot_S> HeaderSlots;

	// Interaction slots for the bottom section of the prompt widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TArray<FGorgeousInteractionPromptSlot_S> HeaderBottomSlots;

	// Interaction slots for the footer section of the prompt widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TArray<FGorgeousInteractionStatEntry_S> Statistics;
	
	// Interaction actions that can be performed when interacting with the actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TArray<FGorgeousInteractionActionPayload_S> InteractionActions;

	// Interaction mode, used to determine the type of interaction (e.g., pickup, use, inspect)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FName InteractionMode = NAME_None;
	
	// Indicates whether the actor can be interacted with, based on its current state and conditions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bCanInteract = false;

	/**
	 * Submits a new header slot to the prompt payload.
	 * 
	 * @param HeaderID The ID of the header section (0 = Top, 1 = Main, 2 = Bottom).
	 * @param SlotName The name of the slot.
	 * @param Title The title text for the slot.
	 * @param Description The description text for the slot.
	 * @param Icon The optional icon for the slot.
	 * @param Payload The optional payload associated with the slot.
	 */
	void SubmitHeaderSlot(const int32 HeaderID, const FName SlotName, const FText& Title, const FText& Description, const TSoftObjectPtr<UTexture2D>& Icon, const FInstancedStruct& Payload)
	{
		FGorgeousInteractionPromptSlot_S NewSlot;
		NewSlot.SlotName = SlotName;
		NewSlot.Title = Title;
		NewSlot.Description = Description;
		NewSlot.Icon = Icon;
		NewSlot.Payload = Payload;

		switch (HeaderID)
		{
			case 0:
				HeaderTopSlots.Add(MoveTemp(NewSlot));
				break;
			case 1:
				HeaderSlots.Add(MoveTemp(NewSlot));
				break;
			case 2:
				HeaderBottomSlots.Add(MoveTemp(NewSlot));
				break;
			default:
				break;
		}
	}
	
	/**
	 * Clears a header slot from the prompt payload based on the provided header ID and slot name.
	 * 
	 * @param HeaderID The ID of the header section (0 = Top, 1 = Main, 2 = Bottom).
	 * @param SlotName The name of the slot to clear.
	 * @return True if a slot was found and removed; false otherwise.
	 */
	bool ClearHeaderSlot(const int32 HeaderID, const FName SlotName)
	{
		TArray<FGorgeousInteractionPromptSlot_S>* TargetArray;

		switch (HeaderID)
		{
		case 0: TargetArray = &HeaderTopSlots; break;
		case 1: TargetArray = &HeaderSlots; break;
		case 2: TargetArray = &HeaderBottomSlots; break;
		default: return false;
		}

		// RemoveAll returns the number of elements removed
		const int32 RemovedCount = TargetArray->RemoveAllSwap([SlotName](const FGorgeousInteractionPromptSlot_S& Slot)
		{
			return Slot.SlotName == SlotName;
		});

		return RemovedCount > 0;
	}

	/**
	 * Submits a new statistic entry to the prompt payload.
	 * 
	 * @param StatTag The gameplay tag associated with the statistic.
	 * @param DisplayLabel The label to display for the statistic.
	 * @param ValueText The value text to display for the statistic.
	 * @param bHideIfEmpty Whether to hide the statistic if the value is empty (default: true).
	 */
	void SubmitStatistic(const FGameplayTag& StatTag, const FText& DisplayLabel, const FText& ValueText, bool bHideIfEmpty = true)
	{
		FGorgeousInteractionStatEntry_S NewStat;
		NewStat.StatTag = StatTag;
		NewStat.DisplayLabel = DisplayLabel;
		NewStat.ValueText = ValueText;
		NewStat.bHideIfEmpty = bHideIfEmpty;
		Statistics.Add(MoveTemp(NewStat));
	}

	/**
	 * Clears a statistic entry from the prompt payload based on the provided tag and label.
	 * 
	 * @param StatTag The gameplay tag associated with the statistic to clear.
	 * @param DisplayLabel The label of the statistic to clear.
	 * @return True if a statistic was found and removed; false otherwise.
	 */
	bool ClearStatistic(const FGameplayTag& StatTag, const FText& DisplayLabel)
	{
		const int32 RemovedCount = Statistics.RemoveAllSwap([&StatTag, &DisplayLabel](const FGorgeousInteractionStatEntry_S& Stat)
			{
				return Stat.StatTag == StatTag && Stat.DisplayLabel.EqualTo(DisplayLabel);
			});

		return RemovedCount > 0;
	}
};
