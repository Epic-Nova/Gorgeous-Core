// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "UObject/Interface.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousExtensionResourceGuardRuntimeInterface.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Runtime contract implemented by Extension Resource Guard assets.
 */
UINTERFACE()
class GORGEOUSCORERUNTIME_API UGorgeousExtensionResourceGuardRuntimeInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * Exposes the identifier and activation requirements declared by an Extension Resource Guard.
 */
class GORGEOUSCORERUNTIME_API IGorgeousExtensionResourceGuardRuntimeInterface
{
    GENERATED_BODY()

public:
    /**
     * Returns the general system identifier protected by the guard.
     *
     * @return The system identifier protected by the guard.
     */
    virtual FName GetGuardSystemIdentifier() const = 0;

    /**
     * Returns required plugin names mapped to their optionality flags.
     *
     * @return The required plugins and their optionality flags.
     */
    virtual const TMap<FName, bool>& GetGuardRequiredPlugins() const = 0;

    /**
     * Returns required blueprint pack identifiers mapped to their optionality flags.
     *
     * @return The required blueprint packs and their optionality flags.
     */
    virtual const TMap<FName, bool>& GetGuardRequiredBlueprintPacks() const = 0;

    /**
     * Returns whether the guarded content is available for activation.
     *
     * @return True when the guarded content is available for activation.
     */
    virtual bool IsGuardContentPresent() const = 0;
};