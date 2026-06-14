// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Insight Matrix (Runtime)                   |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

#pragma once

#include "CoreMinimal.h"

/*
 * NOTE: The Gauntlet controller class (UGorgeousInsightGauntletController) has been disabled
 * because Gauntlet is an experimental plugin that is only available in Program targets,
 * not in Editor builds. UHT does not support conditional UCLASS inheritance.
 * 
 * To use Insight Matrix with Gauntlet automation:
 * 1. Create a separate standalone test program target
 * 2. Enable the Gauntlet plugin for that target  
 * 3. Define GORGEOUSCORE_WITH_GAUNTLET=1 in the Build.cs
 * 4. Include the Gauntlet controller implementation manually
 * 
 * For now, use FGorgeousInsightHarness and UGorgeousInsightMatrixSubsystem directly
 * for programmatic testing without Gauntlet orchestration.
 */

// Stub forward declaration - no actual UCLASS in Editor builds
class UGorgeousInsightGauntletController;
