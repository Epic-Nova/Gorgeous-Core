// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|              Gorgeous Events - Events functionality provider              |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#include "GorgeousBaseWorldContextUObject.h"

//=============================================================================
// UGorgeousBaseWorldContextUObject Implementation
//=============================================================================

UWorld* UGorgeousBaseWorldContextUObject::GetWorld() const
{
    if (!HasAnyFlags(RF_ClassDefaultObject))
    {
        if (UWorld* World = UObject::GetWorld())
        {
            return World;
        }

        if (FallbackOwner.IsValid())
        {
            return FallbackOwner->GetWorld();
        }
    }

    return nullptr;
}
