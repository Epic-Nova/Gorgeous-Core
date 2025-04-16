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
/*======================================================================================================================>
| --------------------------------------------------------------------------------------------------------------------- |
|								TO AVOID MISTAKES READ THIS BEFORE EDITING THIS FILE									|
| --------------------------------------------------------------------------------------------------------------------- |
|								ONLY INCLUDE CLASSES THAT ARE WITHIN THIS MODULE DEFINED								|
|		TO AVOID CIRCULAR DEPENDENCY ERRORS DO NOT USE THIS MINIMAL FILE IN THE "GorgeousCoreRuntime" MODULE			|
<======================================================================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

MONOLITHIC_HEADER_BOILERPLATE()

#include "GorgeousCoreRuntimeGlobals.h"

//Object Variable Includes
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/GorgeousObjectVariableEnums.h"

#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "ObjectVariables/GorgeousObjectVariableDefinitions.h"

#include "ObjectVariables/Helpers/GorgeousObjectVariableHelperMacros.h"

#include "ObjectVariables/Interfaces/GorgeousObjectVariableInteraction_I.h"
#include "ObjectVariables/Interfaces/GorgeousSingleObjectVariablesGetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousSingleObjectVariablesSetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousArrayObjectVariablesGetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousArrayObjectVariablesSetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousMapObjectVariablesGetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousMapObjectVariablesSetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousSetObjectVariablesGetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousSetObjectVariablesSetter_I.h"

//Conditional Object Chooser Includes
#include "ConditionalObjectChoosers/GorgeousConditionalObjectChooser.h"
#include "ConditionalObjectChoosers/GorgeousConditionalObjectChooserEnums.h"
#include "ConditionalObjectChoosers/GorgeousConditionalObjectChooserStructures.h"
#include "ConditionalObjectChoosers/Conditions/GorgeousCondition.h"
#include "ConditionalObjectChoosers/Conditions/GorgeousBooleanCondition.h"
#include "ConditionalObjectChoosers/Conditions/GorgeousGameplayTagCondition.h"
#include "ConditionalObjectChoosers/Conditions/GorgeousIsValidCondition.h"

//Functional Structure Includes
#include "FunctionalStructures/GorgeousFunctionalStructure.h"

//Quality of Life Includes
#include "QualityOfLife/GorgeousGameInstance.h"
#include "QualityOfLife/GorgeousGameMode.h"
#include "QualityOfLife/GorgeousGameState.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "QualityOfLife/GorgeousPlayerState.h"
#include "QualityOfLife/GorgeousWorldSettings.h"