// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "GorgeousStatFoundationStructures.generated.h"

class AGorgeousPlayerController;

/**
 * Global stat value storage for an actor.
 */
USTRUCT(BlueprintType)
struct FGorgeousStatValues_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	TMap<FGameplayTag, float> Values;
};

/**
 * Container for multiple stat values, used by the Stat Storage and its listeners.
 */
UENUM(BlueprintType)
enum class EGorgeousStatRoundingRule : uint8
{
	None,
	RoundToNearest,
	Floor,
	Ceil,
	TwoDecimals
};

/**
 * Defines the category of a stat for UI grouping.
 */
UENUM(BlueprintType)
enum class EGorgeousStatCategory : uint8
{
	Primary,   // e.g. Strength, Agility
	Secondary, // e.g. Armor, Critical Chance
	Resource,  // e.g. Health, Mana, Stamina
	Hidden     // Internal stats not shown to players
};

/**
 * Access rules for a Gorgeous Stat, determining who can read or modify it.
 */
USTRUCT(BlueprintType)
struct FGorgeousStatAccessRules_S
{
	GENERATED_BODY()

	FGorgeousStatAccessRules_S()
		: AccessPolicy(EGorgeousObjectVariableAccessPolicy::Everyone)
	{}

	/** Determines who can receive replicated updates for this stat. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Access")
	EGorgeousObjectVariableAccessPolicy AccessPolicy;

	/** Whitelist of specific controller instances allowed to interact with this stat. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Access", meta = (EditCondition = "AccessPolicy == EGorgeousObjectVariableAccessPolicy::Custom"))
	TArray<TObjectPtr<AGorgeousPlayerController>> AllowedControllers;

	/** Whitelist of controller classes allowed to interact with this stat. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Access", meta = (EditCondition = "AccessPolicy == EGorgeousObjectVariableAccessPolicy::Custom"))
	TArray<TSubclassOf<AGorgeousPlayerController>> AllowedClasses;
};

/**
 * Metadata definition for a Gorgeous Stat.
 */
USTRUCT(BlueprintType)
struct FGorgeousStat_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Identity")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Identity")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Presentation")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Presentation")
	TArray<FLinearColor> RepresentationColors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Values")
	float DefaultValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Values")
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Values")
	float MaxValue = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Values")
	bool bClamped = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Classification")
	EGorgeousStatCategory Category = EGorgeousStatCategory::Primary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Presentation")
	EGorgeousStatRoundingRule RoundingRule = EGorgeousStatRoundingRule::None;

	/** If set, this signal tag will trigger a stat modification when dispatched through the Signal Bridge. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Signals")
	FGameplayTag ModificationSignal;

	/** If true, the modification signal payload is treated as a delta (addition). If false, it's treated as a absolute 'Set' operation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Signals", meta = (EditCondition = "!ModificationSignal.IsNone()"))
	bool bSignalIsDelta = true;

	/** Network access rules for this stat. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Networking")
	FGorgeousStatAccessRules_S AccessRules;
};

/**
 * Payload structure for stat modification signals.
 */
USTRUCT(BlueprintType)
struct FGorgeousStatModificationPayload_S
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float Value = 0.0f;

	/** Optional source of the modification (e.g. the actor that dealt damage). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	TObjectPtr<AActor> Source = nullptr;
};
