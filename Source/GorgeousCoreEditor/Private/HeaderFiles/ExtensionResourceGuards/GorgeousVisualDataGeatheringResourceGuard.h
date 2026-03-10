// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "ExtensionResourceGuard/GorgeousExtensionResourceGuard.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousVisualDataGeatheringResourceGuard.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Extension Resource Guard for the Visual Data Gathering system content pack
 * (Content/Systems/VisualDataGathering).
 */
UCLASS()
class GORGEOUSCOREEDITOR_API UGorgeousVisualDataGatheringResourceGuard : public UGorgeousExtensionResourceGuard
{
	GENERATED_BODY()

public:
	UGorgeousVisualDataGatheringResourceGuard();
};