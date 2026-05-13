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
#include "ConditionalObjectChoosers/Conditions/GorgeousBooleanCondition.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousCoreUtilitiesMinimalShared.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousBooleanCondition Implementation
//=============================================================================

uint8 UGorgeousBooleanCondition::CheckCondition_Implementation()
{
	switch (Mode)
	{
		case EConditionalChooserMode_E::AND:
			return A && B;
		case EConditionalChooserMode_E::OR:
			return A || B;
		case EConditionalChooserMode_E::XOR:
			return A ^ B;
		case EConditionalChooserMode_E::NAND:
			return !(A && B);
		case EConditionalChooserMode_E::NOR:
			return !(A || B);
		case EConditionalChooserMode_E::A_ONLY:
			return A;
		case EConditionalChooserMode_E::B_ONLY:
			return B;
		case EConditionalChooserMode_E::N_A_ONLY:
			return !A;
		case EConditionalChooserMode_E::N_B_ONLY:
			return !B;
	default:
			GT_W_LOG("GT.Core.ConditionalObjectChoosers.Invalid_Mode", TEXT("Invalid mode for UGorgeousBooleanCondition: %d"), static_cast<int32>(Mode));
			return false;
	}
}