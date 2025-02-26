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
#include "ConditionalObjectChoosers/Conditions/GorgeousBooleanCondition.h"

uint8 UGorgeousBooleanCondition::CheckCondition()
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
			UGorgeousLoggingBlueprintFunctionLibrary::LogWarningMessage(FString::Printf(TEXT("Invalid mode for UGorgeousBooleanCondition: %d"), static_cast<int32>(Mode)), "GT.ConditionalObjectChoosers.Invalid_Mode");
			return false;
	}
}
