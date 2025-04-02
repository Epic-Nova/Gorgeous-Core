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
#include "GorgeousSingleton.h"

//=============================================================================
// UGorgeousSingleton Implementation
//=============================================================================

UGorgeousSingleton* UGorgeousSingleton::SingletonInstance = nullptr;

template<typename T>
T* UGorgeousSingleton::GetSingleton()
{
	static_assert(TIsDerivedFrom<T, UGorgeousSingleton>::IsDerived, "The type provided is not derived from UGorgeousSingletonTemplate.");

	if (!SingletonInstance)
	{
		SingletonInstance = NewObject<T>();
		SingletonInstance->AddToRoot();
	}
	return Cast<T>(SingletonInstance);
}

void UGorgeousSingleton::DestroySingleton()
{
	if (SingletonInstance)
	{
		SingletonInstance->RemoveFromRoot();
		SingletonInstance = nullptr;
	}
}
