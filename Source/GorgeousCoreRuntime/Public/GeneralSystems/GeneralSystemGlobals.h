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
#include "GeneralSystemTypes.h"
#include "GorgeousCoreRuntimeGlobals.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GeneralSystemGlobals.generated.h"
//<-------------------------------------------------------------------------->


UCLASS(BlueprintType, DisplayName = "Gorgeous Core Globals")
class UGeneralSystemGlobals : public UGorgeous
{
    GENERATED_BODY()

public:
    
    //@todo: ApplyGorgeousPointDamage() / ApplyGorgeousRadialDamage()
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Damage")
    static float ApplyGorgeousDamageWithInterface(
        AActor* DamagedActor,
        float BaseDamage,
        AController* EventInstigator,
        AActor* DamageCauser,
        TSubclassOf<UDamageType> DamageTypeClass
    );

};
