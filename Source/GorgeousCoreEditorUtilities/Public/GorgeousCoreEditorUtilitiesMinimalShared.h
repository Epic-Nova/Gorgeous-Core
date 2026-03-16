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
/*======================================================================================================================>
| --------------------------------------------------------------------------------------------------------------------- |
|                                  TO AVOID MISTAKES READ THIS BEFORE EDITING THIS FILE                                 |
| --------------------------------------------------------------------------------------------------------------------- |
|                                ONLY INCLUDE CLASSES THAT ARE WITHIN THIS MODULE DEFINED                               |
|          TO AVOID CIRCULAR DEPENDENCY ERRORS DO NOT USE THIS MINIMAL FILE IN THE "GorgeousCoreRuntime" MODULE         |
<======================================================================================================================*/
#pragma once

MONOLITHIC_HEADER_BOILERPLATE()

// Module Core Includes
#include "GorgeousUsefulSettings.h"

// Asset Registration Includes
#include "AssetRegistration/GorgeousAssetRegistrationHelperMacros.h"
#include "AssetRegistration/GorgeousAssetRegistrationStructures.h"
#include "AssetRegistration/GorgeousAssetTypeAction.h"
#include "AssetRegistration/GorgeousFactory.h"

// Library Includes
#include "Libraries/GorgeousEditorLoggingBlueprintFunctionLibrary.h"

// Style Registration Includes
#include "StyleRegistration/GorgeousStyleRegistrationHelperMacros.h"
#include "StyleRegistration/GorgeousStyleRegistrationHelpers.h"

// Validation Includes
#include "Validation/GorgeousValidationHelpers.h"