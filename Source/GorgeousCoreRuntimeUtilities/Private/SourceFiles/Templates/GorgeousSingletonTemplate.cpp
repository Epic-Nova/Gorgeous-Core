/*==========================================================================>
|				Gorgeous Core - Core functionality provider					|
| ------------------------------------------------------------------------- |
|		  Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,			|
|			  administrated by Epic Nova. All rights reserved.				|
| ------------------------------------------------------------------------- |
|					Epic Nova is an independent entity,						|
|	    that is has nothing in common with Epic Games in any capacity.		|
<==========================================================================*/
#include "GorgeousSingletonTemplate.h"

UGorgeousSingletonTemplate* UGorgeousSingletonTemplate::SingletonInstance = nullptr;

template<typename T>
T* UGorgeousSingletonTemplate::GetSingleton()
{
	static_assert(TIsDerivedFrom<T, UGorgeousSingletonTemplate>::IsDerived, "The type provided is not derived from UGorgeousSingletonTemplate.");

	if (!SingletonInstance)
	{
		SingletonInstance = NewObject<T>();
		SingletonInstance->AddToRoot();
	}
	return Cast<T>(SingletonInstance);
}

void UGorgeousSingletonTemplate::DestroySingleton()
{
	if (SingletonInstance)
	{
		SingletonInstance->RemoveFromRoot();
		SingletonInstance = nullptr;
	}
}
