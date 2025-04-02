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

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousConditionalObjectChooserEnums.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Enumeration defining modes for conditional object choosers.
 *
 * Key features include:
 * - Logical operators (AND, OR, XOR, NAND, NOR).
 * - Single input evaluation (A_ONLY, B_ONLY).
 * - Inverted single input evaluation (N_A_ONLY, N_B_ONLY).
 *
 * @author Nils Bergemann
 * @note This enum is used to specify how conditions are evaluated in conditional object choosers.
 */
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

/**
 * Enumeration defining modes for resolving conflicts when multiple gameplay tags are present.
 *
 * Key features include:
 * - Selecting the first, last, or a random tag.
 * - Allowing custom rule-based selection.
 *
 * @author Nils Bergemann
 * @note This enum is used to specify how a single gameplay tag is chosen from a container
 * when multiple tags satisfy a condition.
 */
UENUM(BlueprintType, DisplayName = "Conditional Gameplay Tag Chooser Fight Mode", meta = (ShortTooltip = "Determines the behaviour of how to proceed when more than one gameplay tag is present in the container"))
enum class EConditionalGameplayTagChooserFightMode_E : uint8
{
    FIRST = 0 UMETA(DisplayName = "First", ToolTip = "Returns the first one found from the gameplay tag container."),
    LAST = 1 UMETA(DisplayName = "Last", ToolTip = "Returns the last one found from the gameplay tag container."),
    RANDOM = 2 UMETA(DisplayName = "Random", ToolTip = "Returns a random one from the gameplay tag container."),
    RULE = 3 UMETA(DisplayName = "Rule", ToolTip = "Let's you define a custom rule for your own logic. For this the EvaluateCustomRule function in the condition object must be overwritten, otherwise 0 will be returned.")
};
