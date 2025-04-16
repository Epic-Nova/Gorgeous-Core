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

//Asset Registration
#include "AssetRegistration/GorgeousFactory.h"
#include "AssetRegistration/GorgeousAssetRegistrationStructures.h"
#include "AssetRegistration/GorgeousAssetTypeAction.h"
#include "AssetRegistration/GorgeousAssetRegistration_ES.h"