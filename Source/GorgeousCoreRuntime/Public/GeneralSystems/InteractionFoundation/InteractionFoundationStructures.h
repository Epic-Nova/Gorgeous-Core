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
#include "GeneralSystems/DebugAssist/DebugAssistStructures.h"
#include "GorgeousCoreMinimalShared.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "InteractionFoundationStructures.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Sphere trace configuration used by the Interaction Foundation focus and interact traces.
 *
 * @author Nils Bergemann
 */
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

	// Trace start location.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
	FVector Start = FVector::ZeroVector;

	// Trace end location.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
	FVector End = FVector::ZeroVector;

	// Radius of the swept sphere.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
	float Radius = 15.0f;

	// Collision channel the trace queries against.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_WorldDynamic;

	// Whether to use the complex (per-triangle) collision representation.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
	bool bTraceComplex = false;

	// Whether to ignore the actor that initiated the trace.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
	bool bIgnoreSelf = true;

	// Actors explicitly excluded from the trace.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
	TArray<AActor*> ActorsToIgnore;

	// Visual parameters for debug drawing of the trace.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Foundation")
	FGorgeousDebugAssistVisualParameters DebugVisualParameters;
};

/**
 * Simple interaction prompt payload carrying the text and action context for a single interaction widget.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType, DisplayName = "Gorgeous Interaction Action Payload")
struct GORGEOUSCORERUNTIME_API FGorgeousInteractionActionPayload_S
{
	GENERATED_BODY()

	// Prompt text shown while the action widget is visible.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText InteractionPrompt;

	// Hold duration required before the prompt fires.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float RequiredInteractionTime;

	// Optional action UObject carrying meta context (e.g. for inventory systems).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TSubclassOf<UObject> ActionClass;

	// Forces the prompt widget to redraw by bypassing the refresh system.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bNeedsRefresh;

	FGorgeousInteractionActionPayload_S()
		: InteractionPrompt(FText::FromString("Interact"))
		, RequiredInteractionTime(0.0f)
		, ActionClass(nullptr)
		, bNeedsRefresh(false)
	{}

};

/**
 * A single named slot inside an interaction prompt widget.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType, DisplayName = "Gorgeous Interaction Prompt Slot")
struct GORGEOUSCORERUNTIME_API FGorgeousInteractionPromptSlot_S
{
	GENERATED_BODY()

	// Unique slot identifier used for submission and removal.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FName SlotName = NAME_None;

	// Slot title text.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText Title;

	// Slot description text.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText Description;

	// Optional slot icon.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TSoftObjectPtr<UTexture2D> Icon;

	// Optional instanced payload attached to the slot.
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

/**
 * A single statistic entry shown inside an interaction prompt.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType, DisplayName = "Gorgeous Interaction Stat Entry")
struct GORGEOUSCORERUNTIME_API FGorgeousInteractionStatEntry_S
{
	GENERATED_BODY()

	// Gameplay tag identifying the statistic.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FGameplayTag StatTag;

	// Display label for the statistic.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText DisplayLabel;

	// Value text for the statistic.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText ValueText;

	// Hides the entry when its value is empty.
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
 * Complex interaction prompt payload carrying header, slots, statistics and actions for an interaction widget.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType, DisplayName = "Gorgeous Interaction Prompt Payload")
struct GORGEOUSCORERUNTIME_API FGorgeousInteractionPromptPayload_S
{
	GENERATED_BODY()

	// Main header text of the prompt widget.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText PromptHeader;

	// Secondary descriptor text of the prompt widget.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText PromptDescriptor;

	// Optional icon shown in the prompt widget.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TSoftObjectPtr<UTexture2D> PromptIcon;

	// Interaction tags used for filtering and categorization.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FGameplayTagContainer InteractionTags;

	// Slots rendered in the top header section.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TArray<FGorgeousInteractionPromptSlot_S> HeaderTopSlots;

	// Slots rendered in the main header section.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TArray<FGorgeousInteractionPromptSlot_S> HeaderSlots;

	// Slots rendered in the bottom header section.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TArray<FGorgeousInteractionPromptSlot_S> HeaderBottomSlots;

	// Statistic entries rendered in the prompt footer.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TArray<FGorgeousInteractionStatEntry_S> Statistics;

	// Actions that can be performed when interacting with the actor.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TArray<FGorgeousInteractionActionPayload_S> InteractionActions;

	// Interaction mode (e.g. pickup, use, inspect) derived from the actor state.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FName InteractionMode = NAME_None;

	// Whether the actor can currently be interacted with.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bCanInteract = false;

	/**
	 * Adds a header slot to the prompt payload, routing it to the top, main or bottom section by ID.
	 *
	 * @param HeaderID Section to add to (0 = Top, 1 = Main, 2 = Bottom).
	 * @param SlotName Unique name of the slot.
	 * @param Title Slot title text.
	 * @param Description Slot description text.
	 * @param Icon Optional slot icon.
	 * @param Payload Optional instanced slot payload.
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
	 * Removes a header slot from the prompt payload by section ID and slot name.
	 *
	 * @param HeaderID Section to clear from (0 = Top, 1 = Main, 2 = Bottom).
	 * @param SlotName Name of the slot to remove.
	 * @return True if a slot was found and removed, false otherwise.
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

		// RemoveAllSwap returns the number of elements removed
		const int32 RemovedCount = TargetArray->RemoveAllSwap([SlotName](const FGorgeousInteractionPromptSlot_S& Slot)
		{
			return Slot.SlotName == SlotName;
		});

		return RemovedCount > 0;
	}

	/**
	 * Adds a statistic entry to the prompt payload.
	 *
	 * @param StatTag Gameplay tag identifying the statistic.
	 * @param DisplayLabel Label shown for the statistic.
	 * @param ValueText Value text shown for the statistic.
	 * @param bHideIfEmpty Hides the statistic when its value is empty (default: true).
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
	 * Removes a statistic entry from the prompt payload by tag and label.
	 *
	 * @param StatTag Gameplay tag identifying the statistic.
	 * @param DisplayLabel Label of the statistic to remove.
	 * @return True if a statistic was found and removed, false otherwise.
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