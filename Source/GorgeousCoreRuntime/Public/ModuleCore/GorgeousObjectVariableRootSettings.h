// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "GorgeousObjectVariableRootSettings.generated.h"

USTRUCT(BlueprintType)
struct FGorgeousObjectVariableRootEntry
{
	GENERATED_BODY()

public:
	FGorgeousObjectVariableRootEntry()
		: RootName(NAME_None)
		, RootClass(UGorgeousRootObjectVariable::StaticClass())
		, bSupportsNetworking(true)
		, bEnforceNetworking(false)
		, bHandlesParentlessReplicatedVariables(false)
		, bDefault(false)
	{
	}

	/** Identifier exposed in dropdowns/console commands. */
	UPROPERTY(EditAnywhere, Config, Category = "Root")
	FName RootName;

	/** Concrete root type to instantiate for this entry. */
	UPROPERTY(EditAnywhere, Config, Category = "Root")
	TSoftClassPtr<UGorgeousRootObjectVariable> RootClass;

	/** Whether this root participates in the networking stack. */
	UPROPERTY(EditAnywhere, Config, Category = "Root")
	bool bSupportsNetworking;

	/** If true, every variable registered under this root must expose itself through the network stack and keep replication enabled. */
	UPROPERTY(EditAnywhere, Config, Category = "Root")
	bool bEnforceNetworking;

	/**
	 * Enables shared-root behavior for replicated child variables that temporarily lack a parent/owner.
	 * When true, the root can reparent orphaned variables and accept fallback ownership during replication.
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Root", AdvancedDisplay)
	bool bHandlesParentlessReplicatedVariables;

	/** Marks this entry as the default/fallback root. */
	UPROPERTY(EditAnywhere, Config, Category = "Root")
	bool bDefault;

	/** Optional description surfaced via tooling. */
	UPROPERTY(EditAnywhere, Config, Category = "Root", meta = (MultiLine = true))
	FText Description;
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Core ↪ Object Variable Roots"))
class GORGEOUSCORERUNTIME_API UGorgeousObjectVariableRootSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGorgeousObjectVariableRootSettings(const FObjectInitializer& ObjectInitializer);

	/** List of available roots that can be referenced by name. */
	UPROPERTY(EditAnywhere, Config, Category = "Roots")
	TArray<FGorgeousObjectVariableRootEntry> RegisteredRoots;

	/** Explicit fallback when console commands omit a root argument (optional). */
	UPROPERTY(EditAnywhere, Config, Category = "Roots")
	FName DefaultConsoleRoot;

	static const UGorgeousObjectVariableRootSettings* Get();

	const FGorgeousObjectVariableRootEntry* FindRootEntry(FName RootName) const;

	FName GetDefaultRootName() const;

	TArray<FName> GetRegisteredRootNames() const;

#if WITH_EDITOR
	virtual FName GetCategoryName() const override { return TEXT("Gorgeous Things"); }
	virtual FName GetSectionName() const override { return TEXT("Core|Object Variable Roots"); }
	virtual FText GetSectionText() const override { return NSLOCTEXT("GorgeousRootSettings", "SectionName", "Core ↪ Object Variable Roots"); }
	virtual FText GetSectionDescription() const override { return NSLOCTEXT("GorgeousRootSettings", "SectionDescription", "Define available Object Variable roots and their default networking behavior."); }
#endif
};
