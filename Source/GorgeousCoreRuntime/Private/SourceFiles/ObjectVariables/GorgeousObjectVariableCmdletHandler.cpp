// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#include "ObjectVariables/GorgeousObjectVariableCmdletHandler.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "GorgeousCoreUtilitiesMinimalShared.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousObjectVariableCmdletHandler Implementation
//=============================================================================

namespace
{
	static void TraverseInternal(UGorgeousObjectVariable* Node, const int32 Depth, TFunctionRef<void(UGorgeousObjectVariable*, int32)> Visitor)
	{
		if (!IsValid(Node))
		{
			return;
		}

		Visitor(Node, Depth);
		for (auto& [Key, Child] : Node->VariableRegistry)
		{
			TraverseInternal(Child.Get(), Depth + 1, Visitor);
		}
	}

	static FString DescribeOrphanResolution(const EGorgeousObjectVariableOrphanResolution Mode)
	{
		switch (Mode)
		{
		case EGorgeousObjectVariableOrphanResolution::DestroyOrphans:
			return TEXT("destroy");

		case EGorgeousObjectVariableOrphanResolution::ReparentToRoot:
		default:
			return TEXT("reparent");
		}
	}

	static void AccumulateVariableStats(FGorgeousObjectVariableStats& Stats, UGorgeousObjectVariable* Variable)
	{
		if (!IsValid(Variable))
		{
			return;
		}

		Stats.TotalCount++;
		Stats.PersistentCount += Variable->bPersistent ? 1 : 0;
		Stats.ReplicationActiveCount += Variable->IsReplicationActive() ? 1 : 0;
		Stats.AutoReplicationEligibleCount += Variable->SupportsAutoReplicationFeatures() ? 1 : 0;
		Stats.LegacyRegisteredCount += Variable->IsLegacyReplicationRegistered() ? 1 : 0;
	}

	static bool IsVariableUnderRoot(UGorgeousObjectVariable* Variable, const FName RootName)
	{
		if (!IsValid(Variable))
		{
			return false;
		}

		if (RootName.IsNone())
		{
			return true;
		}

		UGorgeousRootObjectVariable* TargetRoot = UGorgeousRootObjectVariable::GetRootObjectVariable(RootName);
		if (!TargetRoot)
		{
			return false;
		}

		UGorgeousObjectVariable* Current = Variable;
		while (IsValid(Current))
		{
			if (Current == TargetRoot)
			{
				return true;
			}

			Current = Current->GetParent();
		}

		return false;
	}

	static void LogCommandUsage(const TCHAR* CommandLabel, const TCHAR* UsageSuffix, const TCHAR* ContextTag)
	{
		const FString Usage = FString::Printf(TEXT("Usage: %s %s"), CommandLabel, UsageSuffix);
		GT_W_LOG(ContextTag, TEXT("%s"), *Usage);
	}

	static bool ConsumeRootArgument(
		const TCHAR* CommandLabel,
		const TCHAR* UsageSuffix,
		const TArray<FString>& Args,
		int32& ArgIndex,
		FName& OutRootName,
		bool& bOutAllRoots,
		const bool bAllowAllRoots,
		const TCHAR* ContextTag)
	{
		if (Args.Num() <= ArgIndex)
		{
			LogCommandUsage(CommandLabel, UsageSuffix, ContextTag);
			FGorgeousVariableHierarchyHandler::LogAvailableRoots(ContextTag);
			return false;
		}

		bOutAllRoots = false;
		if (!FGorgeousVariableHierarchyHandler::TryParseRootName(Args[ArgIndex], OutRootName, bOutAllRoots))
		{
			GT_W_LOG(ContextTag, TEXT("Invalid root specified: '%s'."), *Args[ArgIndex]);
			FGorgeousVariableHierarchyHandler::LogAvailableRoots(ContextTag);
			return false;
		}

		if (bOutAllRoots && !bAllowAllRoots)
		{
			GT_W_LOG(ContextTag, TEXT("The 'all' root selector is not supported for this command."));
			return false;
		}

		++ArgIndex;
		return true;
	}
}

void FGorgeousVariableHierarchyHandler::Traverse(UGorgeousObjectVariable* Root, TFunctionRef<void(UGorgeousObjectVariable*, int32)> Visitor)
{
	TraverseInternal(Root, 0, Visitor);
}

void FGorgeousVariableHierarchyHandler::TraverseAll(TFunctionRef<void(UGorgeousObjectVariable*, int32)> Visitor)
{
	const TArray<FName> RegisteredRoots = UGorgeousRootObjectVariable::GetRegisteredRootNames();
	if (RegisteredRoots.Num() == 0)
	{
		TraverseRoot(NAME_None, Visitor);
		return;
	}

	for (const FName& RootName : RegisteredRoots)
	{
		TraverseRoot(RootName, Visitor);
	}
}

void FGorgeousVariableHierarchyHandler::TraverseRoot(const FName RootName, TFunctionRef<void(UGorgeousObjectVariable*, int32)> Visitor)
{
	for (UGorgeousObjectVariable* RootEntry : UGorgeousRootObjectVariable::GetRootVariableRegistry(RootName))
	{
		Traverse(RootEntry, Visitor);
	}
}

UGorgeousObjectVariable* FGorgeousVariableHierarchyHandler::ResolveVariableToken(const FString& Token, const FName RootFilter)
{
	if (Token.IsEmpty())
	{
		return nullptr;
	}

	UGorgeousObjectVariable* Result;
	if (FGuid ParsedGuid; FGuid::Parse(Token, ParsedGuid))
	{
		Result = UGorgeousRootObjectVariable::FindVariableByIdentifier(ParsedGuid);
	}
	else
	{
		Result = UGorgeousRootObjectVariable::FindVariableByDisplayName(FName(*Token));
	}

	return IsVariableUnderRoot(Result, RootFilter) ? Result : nullptr;
}

FString FGorgeousVariableHierarchyHandler::DescribeReplicationState(const UGorgeousObjectVariable* Variable)
{
	return (Variable && Variable->IsReplicationActive()) ? TEXT("Active") : TEXT("Disabled");
}

FGorgeousObjectVariableStats FGorgeousVariableHierarchyHandler::GatherStats(UGorgeousObjectVariable* Scope)
{
	FGorgeousObjectVariableStats Stats;
	const auto Visitor = [&Stats](UGorgeousObjectVariable* Variable, int32 /*Depth*/)
	{
		AccumulateVariableStats(Stats, Variable);
	};

	if (Scope)
	{
		Traverse(Scope, Visitor);
	}
	else
	{
		TraverseAll(Visitor);
	}

	return Stats;
}

FGorgeousObjectVariableStats FGorgeousVariableHierarchyHandler::GatherStatsForRoot(const FName RootName)
{
	FGorgeousObjectVariableStats Stats;
	const auto Visitor = [&Stats](UGorgeousObjectVariable* Variable, int32 /*Depth*/)
	{
		AccumulateVariableStats(Stats, Variable);
	};

	TraverseRoot(RootName, Visitor);
	return Stats;
}

bool FGorgeousVariableHierarchyHandler::TryParseRootName(const FString& Token, FName& OutRootName, bool& bOutAllRoots)
{
	bOutAllRoots = false;
	if (Token.IsEmpty())
	{
		return false;
	}

	if (Token.Equals(TEXT("all"), ESearchCase::IgnoreCase))
	{
		OutRootName = NAME_None;
		bOutAllRoots = true;
		return true;
	}

	const FName DefaultRoot = UGorgeousRootObjectVariable::GetDefaultRootName();
	if (Token.Equals(TEXT("default"), ESearchCase::IgnoreCase))
	{
		OutRootName = DefaultRoot;
		return true;
	}

	const TArray<FName> RegisteredRoots = UGorgeousRootObjectVariable::GetRegisteredRootNames();
	if (RegisteredRoots.Num() == 0)
	{
		if (DefaultRoot.ToString().Equals(Token, ESearchCase::IgnoreCase))
		{
			OutRootName = DefaultRoot;
			return true;
		}
		return false;
	}

	const FName Candidate(*Token);
	for (const FName& RootName : RegisteredRoots)
	{
		if (RootName == Candidate || RootName.ToString().Equals(Token, ESearchCase::IgnoreCase))
		{
			OutRootName = RootName;
			return true;
		}
	}

	return false;
}

void FGorgeousVariableHierarchyHandler::LogAvailableRoots(const TCHAR* ContextTag)
{
	const TArray<FName> RegisteredRoots = UGorgeousRootObjectVariable::GetRegisteredRootNames();
	FString RootListing = RegisteredRoots.Num() > 0
		? FString::JoinBy(RegisteredRoots, TEXT(", "), [](const FName& Name) { return Name.ToString(); })
		: FString(TEXT("<none>"));

	GT_I_LOG(ContextTag, TEXT("Available roots: %s"), *RootListing);
	
}

void UGorgeousObjectVariableCmdletHandler::RegisterConsoleCommands()
{
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("gorgeous.ov.list"),
		TEXT("Lists all gorgeous object variables in a hierarchy."),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UGorgeousObjectVariableCmdletHandler::ListGorgeousVariables),
		ECVF_Default
	);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("gorgeous.ov.lookup"),
		TEXT("Finds a Gorgeous Object Variable by display name or identifier."),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UGorgeousObjectVariableCmdletHandler::LookupGorgeousVariable),
		ECVF_Default
	);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("gorgeous.ov.stats"),
		TEXT("Shows replication stats for all Gorgeous Object Variables or a specific subtree."),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UGorgeousObjectVariableCmdletHandler::PrintGorgeousVariableStats),
		ECVF_Default
	);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("gorgeous.ov.orphans"),
		TEXT("Gets or sets how orphaned Gorgeous Object Variables are handled (reparent|destroy)."),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UGorgeousObjectVariableCmdletHandler::ConfigureOrphanBehavior),
		ECVF_Default
	);
}

void UGorgeousObjectVariableCmdletHandler::ListGorgeousVariables(const TArray<FString>& Args)
{
	int32 ArgIndex = 0;
	FName RootName = NAME_None;
	bool bAllRoots = false;
	if (!ConsumeRootArgument(TEXT("gorgeous.ov.list"), TEXT("<RootName|all>"), Args, ArgIndex, RootName, bAllRoots, true, TEXT("GorgeousVariableHierarchy")))
	{
		return;
	}

	const auto Visitor = [](UGorgeousObjectVariable* Variable, const int32 IndentLevel)
	{
		const FString ReplicationState = FGorgeousVariableHierarchyHandler::DescribeReplicationState(Variable);
		const FString Indent = FString::ChrN(IndentLevel * 4, ' ');
		const FString LogMessage = FString::Printf(TEXT("%s- %s (%s) --- Outer: %s [Replication: %s]"),
			*Indent,
			*Variable->GetDisplayNameOrFallback(),
			*Variable->UniqueIdentifier.ToString(),
			*(Variable->GetOuter() ? Variable->GetOuter()->GetName() : TEXT("")),
			*ReplicationState);
		
		GT_I_LOG(TEXT("GT.ObjectVariables.HierarchyTraversal"), TEXT("%s: Visited variable %s with identifier %s at depth %d."), *LogMessage, *Variable->GetDisplayNameOrFallback(), *Variable->UniqueIdentifier.ToString(), IndentLevel);
	};

	if (bAllRoots)
	{
		FGorgeousVariableHierarchyHandler::TraverseAll(Visitor);
	}
	else
	{
		FGorgeousVariableHierarchyHandler::TraverseRoot(RootName, Visitor);
	}
}

void UGorgeousObjectVariableCmdletHandler::LookupGorgeousVariable(const TArray<FString>& Args)
{
	int32 ArgIndex = 0;
	FName RootName = NAME_None;
	bool bAllRoots = false;
	if (!ConsumeRootArgument(TEXT("gorgeous.ov.lookup"), TEXT("<RootName> <DisplayName|Identifier>"), Args, ArgIndex, RootName, bAllRoots, false, TEXT("GorgeousVariableLookup")))
	{
		return;
	}

	if (Args.Num() <= ArgIndex)
	{
		LogCommandUsage(TEXT("gorgeous.ov.lookup"), TEXT("<RootName> <DisplayName|Identifier>"), TEXT("GorgeousVariableLookup"));
		return;
	}

	const FString& Query = Args[ArgIndex];
	UGorgeousObjectVariable* Result = FGorgeousVariableHierarchyHandler::ResolveVariableToken(Query, RootName);

	if (!Result)
	{
		GT_W_LOG(TEXT("GT.ObjectVariables.LookupFailed"), TEXT("Lookup failed for query '%s' under root %s."), *Query, *RootName.ToString());
		return;
	}

	const FString ReplicationState = FGorgeousVariableHierarchyHandler::DescribeReplicationState(Result);
	const FString ParentLabel = Result->GetParent() ? Result->GetParent()->GetDisplayNameOrFallback() : FString(TEXT("<root>"));
	const FString Report = FString::Printf(TEXT("Name: %s | Identifier: %s | Parent: %s | Replication: %s | Persistent: %s"),
		*Result->GetDisplayNameOrFallback(),
		*Result->UniqueIdentifier.ToString(),
		*ParentLabel,
		*ReplicationState,
		Result->bPersistent ? TEXT("Yes") : TEXT("No"));

	GT_I_LOG(TEXT("GT.ObjectVariables.Lookup"), TEXT("Lookup successful for query '%s' under root %s."), *Query, *RootName.ToString());
}

void UGorgeousObjectVariableCmdletHandler::PrintGorgeousVariableStats(const TArray<FString>& Args)
{
	int32 ArgIndex = 0;
	FName RootName = NAME_None;
	bool bAllRoots = false;
	if (!ConsumeRootArgument(TEXT("gorgeous.ov.stats"), TEXT("<RootName> [DisplayName|Identifier]"), Args, ArgIndex, RootName, bAllRoots, false, TEXT("GorgeousVariableStats")))
	{
		return;
	}

	UGorgeousObjectVariable* Scope = nullptr;
	if (Args.Num() > ArgIndex)
	{
		Scope = FGorgeousVariableHierarchyHandler::ResolveVariableToken(Args[ArgIndex], RootName);
		if (!Scope)
		{
			GT_W_LOG("GT.ObjectVariables.Stats.LookupFailed", TEXT("Failed to find variable for stats scope query '%s' under root %s."), *Args[ArgIndex], *RootName.ToString());
			return;
		}
	}

	const FGorgeousObjectVariableStats Stats = Scope
		? FGorgeousVariableHierarchyHandler::GatherStats(Scope)
		: FGorgeousVariableHierarchyHandler::GatherStatsForRoot(RootName);
	if (Stats.TotalCount == 0)
	{
		GT_I_LOG("GT.ObjectVariables.Stats.NoVariables", TEXT("No variables found for stats query under root %s."), *RootName.ToString());
		return;
	}

	const FString ScopeLabel = Scope ? Scope->GetDisplayNameOrFallback() : FString::Printf(TEXT("Root %s"), *RootName.ToString());
	const FString Report = FString::Printf(TEXT("%s -> Total: %d | Persistent: %d | AutoReplication: %d | Active: %d | Legacy: %d"),
		*ScopeLabel,
		Stats.TotalCount,
		Stats.PersistentCount,
		Stats.AutoReplicationEligibleCount,
		Stats.ReplicationActiveCount,
		Stats.LegacyRegisteredCount);
	
	GT_I_LOG("GT.ObjectVariables.Stats", TEXT("Compiled stats for scope %s."), *ScopeLabel);
}

void UGorgeousObjectVariableCmdletHandler::ConfigureOrphanBehavior(const TArray<FString>& Args)
{
	const EGorgeousObjectVariableOrphanResolution CurrentMode = UGorgeousRootObjectVariable::GetDefaultOrphanResolution();
	if (Args.Num() == 0)
	{
		GT_I_LOG("GT.ObjectVariables.OrphanPolicy.Current", TEXT("Queried current orphan policy %s."), *DescribeOrphanResolution(CurrentMode));
		return;
	}

	FString ModeString = Args[0];
	ModeString.ToLowerInline();
	EGorgeousObjectVariableOrphanResolution DesiredMode;
	if (ModeString == TEXT("reparent"))
	{
		DesiredMode = EGorgeousObjectVariableOrphanResolution::ReparentToRoot;
	}
	else if (ModeString == TEXT("destroy"))
	{
		DesiredMode = EGorgeousObjectVariableOrphanResolution::DestroyOrphans;
	}
	else
	{
		GT_W_LOG("GT.ObjectVariables.OrphanPolicy.InvalidArgument", TEXT("Invalid argument '%s' for gorgeous.ov.orphans command; expected 'reparent' or 'destroy'."), *Args[0]);
		return;
	}

	if (DesiredMode == CurrentMode)
	{
		GT_I_LOG("GT.ObjectVariables.OrphanPolicy.Unchanged", TEXT("Orphan policy is already set to %s."), *DescribeOrphanResolution(CurrentMode));
		return;
	}

	UGorgeousRootObjectVariable::SetDefaultOrphanResolution(DesiredMode);
	GT_S_LOG("GT.ObjectVariables.OrphanPolicy.Changed", TEXT("Changed orphan policy from %s to %s."), *DescribeOrphanResolution(CurrentMode), *DescribeOrphanResolution(DesiredMode));
}