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

#include "CoreMinimal.h"
#include "GorgeousConditionalObjectChooserEnums.generated.h"

UENUM(BlueprintType, DisplayName = "Coniditional Chooser Mode", meta = (ShortTooltip = "The mode in what the conditonal check should perform"))
enum class EConditionalChooserMode_E : uint8
{
	AND = 0 UMETA(DisplayName = "AND", ToolTip = "A and B must be true/valid."),
	OR = 1 UMETA(DisplayName = "OR", ToolTip = "Either A or B (or both) must be true/valid."),
	XOR = 2 UMETA(DisplayName = "XOR", ToolTip = "Either A or B must be true/valid, but not both."),
	NAND = 3 UMETA(DisplayName = "NAND", ToolTip = "A and B must NOT both be true/valid."),
	NOR = 4 UMETA(DisplayName = "NOR", ToolTip = "Neither A nor B can be true/valid."),
	A_ONLY = 5 UMETA(DisplayName = "A Only", ToolTip = "Only A is evaluated."),
	B_ONLY = 6 UMETA(DisplayName = "B Only", ToolTip = "Only B is evaluated."),
	
	N_A_ONLY = 7 UMETA(DisplayName = "Not A Only", ToolTip = "Only A is evaluated and then inverted."),
	N_B_ONLY = 8 UMETA(DisplayName = "Not B Only", ToolTip = "Only B is evaluated and then inverted."),
};
