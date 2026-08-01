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
#include "Templates/Function.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousObjectVariableCmdletHandler.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UGorgeousObjectVariable;
//<------------------------------------------------------------->

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

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Object Variable Cmdlet Handler
| Functional Name: UGorgeousObjectVariableCmdletHandler
| Parent Class: UObject
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Handles console commands related to Gorgeous Object Variables.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ObjectVariables/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ObjectVariables/GorgeousObjectVariableCmdletHandler",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/ObjectVariables/Examples/"
		)
)
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
