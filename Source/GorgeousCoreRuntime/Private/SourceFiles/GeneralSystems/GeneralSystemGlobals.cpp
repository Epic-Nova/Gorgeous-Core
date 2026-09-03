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
#include "GeneralSystems/GeneralSystemGlobals.h"
#include "Engine/DamageEvents.h"

float UGeneralSystemGlobals::ApplyGorgeousDamageWithInterface(
    AActor* DamagedActor,
    float BaseDamage,
    AController* EventInstigator,
    AActor* DamageCauser,
    TSubclassOf<UDamageType> DamageTypeClass
)
{
    if (!DamagedActor)
    {
        return 0.0f;
    }

    FGorgeousDamageContext_S Context;

    Context.DamagedActor = DamagedActor;
    Context.BaseDamage = BaseDamage;
    Context.EventInstigator = EventInstigator;
    Context.DamageCauser = DamageCauser;
    Context.DamageTypeClass = DamageTypeClass;

    // DamageTypeClass als Objekt behandeln
    if (UDamageType* DamageType = DamageTypeClass
        ? DamageTypeClass->GetDefaultObject<UDamageType>()
        : nullptr)
    {
        if (DamageType->GetClass()->ImplementsInterface(
                UGorgeousDamageTypeHandler_I::StaticClass()))
        {
            IGorgeousDamageTypeHandler_I::Execute_HandleDamage(
                DamageType,
                Context
            );
        }
    }

    FDamageEvent DamageEvent;
    DamageEvent.DamageTypeClass = Context.DamageTypeClass;

    return Context.DamagedActor->TakeDamage(
        Context.BaseDamage,
        DamageEvent,
        Context.EventInstigator,
        Context.DamageCauser
    );
}
