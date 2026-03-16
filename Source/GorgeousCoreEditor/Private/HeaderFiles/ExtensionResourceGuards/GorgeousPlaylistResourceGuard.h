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
#include "GorgeousPlaylistResourceGuard.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Extension Resource Guard for the Playlist system content pack
 * (Content/Systems/Playlist).
 */
UCLASS()
class GORGEOUSCOREEDITOR_API UGorgeousPlaylistResourceGuard : public UGorgeousExtensionResourceGuard
{
	GENERATED_BODY()

public:
	UGorgeousPlaylistResourceGuard();
};