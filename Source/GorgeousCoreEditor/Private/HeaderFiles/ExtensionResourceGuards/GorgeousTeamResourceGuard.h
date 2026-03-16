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
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "ExtensionResourceGuard/GorgeousExtensionResourceGuard.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousTeamResourceGuard.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Extension Resource Guard for the Team system content pack
 * (Content/Systems/Team).
 */
UCLASS()
class GORGEOUSCOREEDITOR_API UGorgeousTeamResourceGuard : public UGorgeousExtensionResourceGuard
{
	GENERATED_BODY()

public:
	UGorgeousTeamResourceGuard();
};