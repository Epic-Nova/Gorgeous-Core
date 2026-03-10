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
#include "ExtensionResourceGuards/GorgeousVisualDataGatheringResourceGuard.h"

//=============================================================================
// UGorgeousVisualDataGatheringResourceGuard Implementation
//=============================================================================

UGorgeousVisualDataGatheringResourceGuard::UGorgeousVisualDataGatheringResourceGuard()
{
	SystemIdentifier = FName("VisualDataGathering");
	SystemDisplayName = NSLOCTEXT("GorgeousExtensionResourceGuard", "VisualDataGatheringSystem", "Visual Data Gathering System");
	RequiredPlugins = { FName("CommonUI") };
	OwningPluginName = FName("GorgeousCore");
	ContentSubPath = TEXT("Systems/VisualDataGathering");
}