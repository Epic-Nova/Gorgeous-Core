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
#include "ConditionalObjectChoosers/Conditions/GorgeousIsValidCondition.h"

//<=============================--- Includes ---=============================>
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousCoreUtilitiesMinimalShared.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousIsValidCondition Implementation
//=============================================================================
uint8 UGorgeousIsValidCondition::CheckCondition_Implementation()
{
	switch (Mode)
	{
		case EConditionalChooserMode_E::AND:
			return A != nullptr && B != nullptr;
		case EConditionalChooserMode_E::OR:
			return A != nullptr || B != nullptr;
		case EConditionalChooserMode_E::XOR:
			return (A != nullptr) ^ (B != nullptr);
		case EConditionalChooserMode_E::NAND:
			return !(A != nullptr && B != nullptr);
		case EConditionalChooserMode_E::NOR:
			return !(A != nullptr || B != nullptr);
		case EConditionalChooserMode_E::A_ONLY:
			return A != nullptr;
		case EConditionalChooserMode_E::B_ONLY:
			return B != nullptr;
		case EConditionalChooserMode_E::N_A_ONLY:
			return A == nullptr;
		case EConditionalChooserMode_E::N_B_ONLY:
			return B == nullptr;
		default:
			UGorgeousLoggingBlueprintFunctionLibrary::LogWarningMessage(FString::Printf(TEXT("Invalid mode for UGorgeousIsValidCondition: %d"), static_cast<int32>(Mode)), "GT.ConditionalObjectChoosers.Invalid_Mode");
			return false;
	}
}
