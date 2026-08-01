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
#include "ConditionalObjectChoosers/Conditions/GorgeousCondition.h"

//<=============================--- Includes ---=============================>
//<-------------------------=== Module Includes ===-------------------------->
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousCondition Implementation
//=============================================================================

uint8 UGorgeousCondition::CheckCondition_Implementation()
{
	GT_W_LOG("GT.Core.ConditionalObjectChoosers.CheckCondition_Not_Implemented", TEXT("CheckCondition not implemented for %s. Returning 0 by default."), *GetClass()->GetName());
	return 0;
}