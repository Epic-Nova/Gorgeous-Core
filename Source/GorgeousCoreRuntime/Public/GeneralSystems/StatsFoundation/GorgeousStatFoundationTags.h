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
//<--------------------------=== Engine Includes ===------------------------->
#include "NativeGameplayTags.h"
//<-------------------------------------------------------------------------->

/**
 * System-defined gameplay tags for the Gorgeous Stat Foundation.
 *
 * @author Nils Bergemann
 */

// Signals emitted when statistic values change or are restored.
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_Signal_Stat_Changed);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_Signal_Stat_Restored);