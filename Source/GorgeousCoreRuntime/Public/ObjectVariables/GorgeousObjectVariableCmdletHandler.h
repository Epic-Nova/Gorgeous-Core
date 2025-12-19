// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<-------------------------=== Module Includes ===-------------------------->
#include "Templates/Function.h"
#include "GorgeousObjectVariableCmdletHandler.generated.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
//<-------------------------------------------------------------------------->

class UGorgeousObjectVariable;

/** Aggregated replication stats for a scope of object variables. */
struct FGorgeousObjectVariableStats
{
	int32 TotalCount = 0;
	int32 PersistentCount = 0;
	int32 ReplicationActiveCount = 0;
	int32 AutoReplicationEligibleCount = 0;
	int32 LegacyRegisteredCount = 0;
};

/** Lightweight helper for traversing and querying Gorgeous Object Variable hierarchies. */
struct FGorgeousVariableHierarchyHandler
{
	static void Traverse(UGorgeousObjectVariable* Root, TFunctionRef<void(UGorgeousObjectVariable*, int32)> Visitor);
	static void TraverseAll(TFunctionRef<void(UGorgeousObjectVariable*, int32)> Visitor);
	static void TraverseRoot(FName RootName, TFunctionRef<void(UGorgeousObjectVariable*, int32)> Visitor);
	static UGorgeousObjectVariable* ResolveVariableToken(const FString& Token, FName RootFilter = NAME_None);
	static FString DescribeReplicationState(const UGorgeousObjectVariable* Variable);
	static FGorgeousObjectVariableStats GatherStats(UGorgeousObjectVariable* Scope);
	static FGorgeousObjectVariableStats GatherStatsForRoot(FName RootName);
	static bool TryParseRootName(const FString& Token, FName& OutRootName, bool& bOutAllRoots);
	static void LogAvailableRoots(const TCHAR* ContextTag);
};

/**
 * Handles console commands related to Gorgeous Object Variables.
 *
 * @note This class provides a way to interact with Gorgeous Object Variables through the console.
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousObjectVariableCmdletHandler : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Registers console commands related to Gorgeous Object Variables.
	 */
	static void RegisterConsoleCommands();

	/**
	 * Lists Gorgeous Variables based on the provided arguments.
	 *
	 * @param Args The arguments provided to the console command.
	 */
	static void ListGorgeousVariables(const TArray<FString>& Args);

	/**
	 * Looks up a specific variable by display name or identifier.
	 */
	static void LookupGorgeousVariable(const TArray<FString>& Args);

	/**
	 * Prints replication stats for the entire registry or a specific variable subtree.
	 */
	static void PrintGorgeousVariableStats(const TArray<FString>& Args);

	/**
	 * Configures orphan-handling behavior for the root registry.
	 */
	static void ConfigureOrphanBehavior(const TArray<FString>& Args);
};