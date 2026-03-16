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
#include "AutoReplication/GorgeousAutoReplicationTypes.h"
#include "ReplicationGraph.h"
#include "Engine/DeveloperSettings.h"
#include "GorgeousAutoReplicationSettings.generated.h"

class UGorgeousAutoReplicationGraph;
class UReplicationGraph;

/** Centralized developer settings that configure the Auto Replication backends. */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Core ↪ Auto Replication"))
class GORGEOUSCORERUNTIME_API UGorgeousAutoReplicationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGorgeousAutoReplicationSettings(const FObjectInitializer& ObjectInitializer);

	/** True if Iris should be initialized for object variables when available. */
	UPROPERTY(Config, EditAnywhere, Category = "Backends", meta = (EditCondition = "!bEnableReplicationGraph", EditConditionHides))
	bool bEnableIris;

	/** True if a dedicated replication grap<<h should be created for auto-replicated objects. */
	UPROPERTY(Config, EditAnywhere, Category = "Backends", meta = (EditCondition = "!bEnableIris", EditConditionHides))
	bool bEnableReplicationGraph;

	/** Optional class that overrides the default auto-replication graph implementation. */
	UPROPERTY(Config, EditAnywhere, Category = "Backends", meta = (EditCondition = "bEnableReplicationGraph", EditConditionHides))
	TSoftClassPtr<UReplicationGraph> AutoReplicationGraphClass;

	/** If true, apply an explicit runtime override to enable Iris regardless of other runtime toggles. */
	UPROPERTY(Config, EditAnywhere, Category = "Backends", meta = (DisplayName = "Force Iris At Runtime", EditCondition = "bEnableIris", EditConditionHides))
	bool bForceIrisAtRuntime;

	/** If true, apply an explicit runtime override to enable the replication graph regardless of other runtime toggles. */
	UPROPERTY(Config, EditAnywhere, Category = "Backends", meta = (DisplayName = "Force Replication Graph At Runtime", EditCondition = "bEnableReplicationGraph", EditConditionHides))
	bool bForceReplicationGraphAtRuntime;

     /** Default stream configuration applied to every object variable unless overridden. */
     UPROPERTY(Config, EditAnywhere, Category = "Streams")
     FGorgeousAutoReplicationStreamConfig DefaultStreamConfig;

	/** Per-object overrides referenced by AutoReplication key. */
	UPROPERTY(Config, EditAnywhere, Category = "Streams")
	TMap<FName, FGorgeousAutoReplicationStreamConfig> StreamOverrides;

	static const UGorgeousAutoReplicationSettings* Get();

#if WITH_EDITOR
	virtual FName GetCategoryName() const override { return TEXT("Gorgeous Things"); }
	virtual FName GetSectionName() const override { return TEXT("Core|Auto Replication"); }
	virtual FText GetSectionText() const override { return NSLOCTEXT("GorgeousAutoReplication", "SectionName", "Core ↪ Auto Replication"); }
	virtual FText GetSectionDescription() const override { return NSLOCTEXT("GorgeousAutoReplication", "SectionDescription", "Configure Iris and Replication Graph backends for auto-replicated objects."); }
#endif
};
