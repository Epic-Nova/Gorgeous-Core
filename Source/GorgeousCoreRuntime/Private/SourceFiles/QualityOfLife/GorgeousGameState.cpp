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
#include "QualityOfLife/GorgeousGameState.h"

//<=============================--- Includes ---=============================>
//<-------------------------=== Module Includes ===-------------------------->
#include "QualityOfLife/Helpers/GorgeousQualityOfLIfeHelperMacros.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// AGorgeousGameState Implementation
//=============================================================================

void AGorgeousGameState::BeginPlay()
{
	UE_DECLARE_QOF_CLASS_INIT_INVOKE_ADDITIONAL_DATA
	
	Super::BeginPlay();
}

#if WITH_EDITOR
UE_DECLARE_QOF_CLASS_POST_EDIT_CHANGE_PROPERTY(AGorgeousGameState)
#endif WITH_EDITOR
