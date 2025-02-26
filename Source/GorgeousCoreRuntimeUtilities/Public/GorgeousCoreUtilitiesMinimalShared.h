/*==========================================================================>
|				Gorgeous Core - Core functionality provider					|
| ------------------------------------------------------------------------- |
|		  Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,			|
|			  administrated by Epic Nova. All rights reserved.				|
| ------------------------------------------------------------------------- |
|					Epic Nova is an independent entity,						|
|	    that is has nothing in common with Epic Games in any capacity.		|
<==========================================================================*/
/*======================================================================================================================>
| --------------------------------------------------------------------------------------------------------------------- |
|								TO AVOID MISTAKES READ THIS BEFORE EDITING THIS FILE									|
| --------------------------------------------------------------------------------------------------------------------- |
|							ONLY INCLUDE CLASSES THAT ARE WITHIN THIS MODULE DEFINED                                    |
|	TO AVOID CIRCULAR DEPENDENCY ERRORS DO NOT USE THIS MINIMAL FILE IN THE "GorgeousCoreRuntimeUtilities" MODULE       |
<======================================================================================================================*/
#pragma once

#include "GorgeousCoreRuntimeUtilitiesEnums.h"
#include "GorgeousCoreRuntimeUtilitiesGlobals.h"
#include "GorgeousCoreRuntimeUtilitiesLogging.h"
#include "GorgeousCoreRuntimeUtilitiesMacros.h"
#include "GorgeousCoreRuntimeUtilitiesStructures.h"

//Helper Includes
#include "Helpers/GorgeousConversationHelper.h"
#include "Helpers/GorgeousDatabaseHelper.h"
#include "Helpers/GorgeousDirectoryHelper.h"
#include "Helpers/GorgeousFileHelper.h"
#include "Helpers/GorgeousLanguageHelper.h"
#include "Helpers/GorgeousLoggingHelper.h"

//Macro Helper Includes
#include "Helpers/Macros/GorgeousConnectionHelperMacros.h"
#include "Helpers/Macros/GorgeousExtensionHelperMacros.h"
#include "Helpers/Macros/GorgeousPlatformHelperMacros.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"

//Library Includes
#include "Libraries/GorgeousLoggingBlueprintFunctionLibrary.h"

//Template Includes
#include "Templates/GorgeousSingletonTemplate.h"