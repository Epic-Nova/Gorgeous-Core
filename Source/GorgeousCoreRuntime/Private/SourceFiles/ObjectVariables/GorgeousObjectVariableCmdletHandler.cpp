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
#include "ObjectVariables/GorgeousObjectVariableCmdletHandler.h"

//<=============================--- Includes ---=============================>
//<-------------------------=== Module Includes ===-------------------------->
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "GorgeousCoreUtilitiesMinimalShared.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousObjectVariableCmdletHandler Implementation
//=============================================================================

void UGorgeousObjectVariableCmdletHandler::RegisterConsoleCommands()
{
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("gorgeous.ov.list"),
		TEXT("Lists all gorgeous object variables in a hierarchy."),
		FConsoleCommandWithArgsDelegate::CreateStatic(&UGorgeousObjectVariableCmdletHandler::ListGorgeousVariables),
		ECVF_Default
	);
}

void UGorgeousObjectVariableCmdletHandler::ListGorgeousVariables(const TArray<FString>& Args)
{
	std::function<void(UGorgeousObjectVariable*, int32)> PrintEntry;
	PrintEntry = [&PrintEntry](UGorgeousObjectVariable* Variable, const int32 IndentLevel)
	{
		if (!Variable) return;

		const FString Indent = FString::ChrN(IndentLevel * 4, ' ');
		const FString LogMessage = FString::Printf(TEXT("%s- %s"), *Indent, *Variable->UniqueIdentifier.ToString());

		UGorgeousLoggingBlueprintFunctionLibrary::LogInformationMessage(LogMessage, TEXT("GorgeousVariableHierarchy"), 5.0f, false, true, GWorld);

		for (TObjectPtr Child : Variable->VariableRegistry)
		{
			PrintEntry(Child, IndentLevel + 1);
		}
	};

	for (UGorgeousObjectVariable* RootEntry : UGorgeousRootObjectVariable::GetRootVariableRegistry())
	{
		PrintEntry(RootEntry, 0);
	}
}