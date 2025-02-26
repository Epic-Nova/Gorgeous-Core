// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|      that is has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/
#pragma once

#include "GorgeousObjectVariableCmdletHandler.generated.h"

UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousObjectVariableCmdletHandler : public UObject
{
	GENERATED_BODY()

public:
	static void RegisterConsoleCommands();

	static void ListGorgeousVariables(const TArray<FString>& Args);
};