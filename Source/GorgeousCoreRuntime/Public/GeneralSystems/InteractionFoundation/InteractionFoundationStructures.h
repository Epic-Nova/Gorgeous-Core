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
 * Promt payload returned by world items through the interaction foundation interface.
 * Represents a simple interaction widget.
 */
USTRUCT(Blueprintable, DisplayName = "Gorgeous Interaction Action Payload")
struct GORGEOUSCORERUNTIME_API FGorgeousInteractionActionPayload_S
{
    GENERATED_BODY()
    
    // Text to display when the action widget is visible
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FText InteractionPromt;
    
    // Required hold time for this promt to fire
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float RequiredInteractionTime;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TSubclassOf<UObject> ActionClass;
	
    // Refresh flag to indicate bypass of the refresh system to forcefully redraw the promt widget
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bNeedsRefresh;
    
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
};

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
};

/**
 * Prompt payload returned by world items through the interaction foundation interface.
 * Represents a complex interaction widget with multiple slots and statistics.
 */
USTRUCT(BlueprintType, Blueprintable, DisplayName = "Gorgeous Inventory Interaction Prompt Payload")
struct GORGEOUSCORERUNTIME_API FGorgeousInteractionPromptPayload_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText ItemDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TSoftObjectPtr<UTexture2D> ItemIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FGameplayTagContainer InteractionTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TArray<FGorgeousInteractionPromptSlot_S> HeaderTopSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TArray<FGorgeousInteractionPromptSlot_S> HeaderSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TArray<FGorgeousInteractionPromptSlot_S> HeaderBottomSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TArray<FGorgeousInteractionStatEntry_S> Statistics;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TArray<FGorgeousInteractionActionPayload_S> InteractionActions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bCanInteract = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FName InteractionMode = NAME_None;
};
