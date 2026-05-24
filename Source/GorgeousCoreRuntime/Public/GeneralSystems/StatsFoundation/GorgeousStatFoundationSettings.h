// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GorgeousStatFoundationStructures.h"
#include "GorgeousStatFoundationSettings.generated.h"

/**
 * Developer Settings for the Gorgeous Stat System.
 */
UCLASS(Config = GorgeousStats, DefaultConfig, meta = (DisplayName = "Core ↪ Stats"))

class GORGEOUSCORERUNTIME_API UGorgeousStatFoundationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGorgeousStatFoundationSettings();

	/** Global registry of all available stats in the project. */
	UPROPERTY(Config, EditAnywhere, Category = "Stat Registry", meta = (ForceInlineRow))
	TMap<FGameplayTag, FGorgeousStat_S> StatRegistry;

	/** If true, the system will automatically dispatch a Signal Bridge signal when a stat changes. */
	UPROPERTY(Config, EditAnywhere, Category = "System Configuration")
	bool bAutoDispatchChangeSignals = true;

	/** The root tag used for auto-dispatched change signals (e.g., 'Signal.StatChanged'). */
	UPROPERTY(Config, EditAnywhere, Category = "System Configuration", meta = (EditCondition = "bAutoDispatchChangeSignals"))
	FGameplayTag ChangeSignalRoot;

	/** The signal tag dispatched when stats are restored from the network root. */
	UPROPERTY(Config, EditAnywhere, Category = "System Configuration")
	FGameplayTag RestoredSignal;

	/** 
	 * Find stat metadata by tag.
	 */
	const FGorgeousStat_S* FindStatMetadata(const FGameplayTag& Tag) const;

#if WITH_EDITOR
	virtual FName GetCategoryName() const override { return TEXT("Gorgeous Things"); }
	virtual FName GetSectionName() const override { return TEXT("Core|Stats"); }
	virtual FText GetSectionText() const override { return NSLOCTEXT("GorgeousStatSettings", "SectionName", "Core ↪ Stats"); }
	virtual FText GetSectionDescription() const override { return NSLOCTEXT("GorgeousStatSettings", "SectionDescription", "Configure global stat registry, signal routing, and default values."); }
#endif
};

