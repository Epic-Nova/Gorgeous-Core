// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|              Gorgeous Events - Events functionality provider              |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
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
	 
        if (CachedOwner)
        {
            return CachedOwner->GetWorld();
        }
	 
        if (const UObject* Outer = GetOuter())
        {
            return Outer->GetWorld();
        }
    }

    return nullptr;
}
