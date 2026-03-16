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
#include "ExtensionResourceGuards/GorgeousTeamResourceGuard.h"

//=============================================================================
// UGorgeousTeamResourceGuard Implementation
//=============================================================================

UGorgeousTeamResourceGuard::UGorgeousTeamResourceGuard()
{
	SystemIdentifier = FName("Team");
	SystemDisplayName = NSLOCTEXT("GorgeousExtensionResourceGuard", "TeamSystem", "Team System");
	RequiredPlugins = { FName("CommonUI") };
	OwningPluginName = FName("GorgeousCore");
	ContentSubPath = TEXT("Systems/Team");
}