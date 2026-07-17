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

//<=============================--- Includes ---============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//<-------------------------------------------------------------------------->

/**
 * Registers dotted console aliases for Gorgeous cheat-manager extensions.
 *
 * @author Nils Bergemann
 */
class GORGEOUSCORERUNTIME_API FGorgeousCheatCommandAliases
{
public:

	// Registers the dotted console aliases once the runtime module starts.
	static void RegisterConsoleCommands();
};