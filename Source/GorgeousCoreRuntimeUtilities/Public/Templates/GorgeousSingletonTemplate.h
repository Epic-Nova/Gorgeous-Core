// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|      that is has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/
#pragma once

#include "GorgeousSingletonTemplate.generated.h"

/**
 * Used to provide a singleton way of accessing a class that depends on this template.
 */
UCLASS()
class GORGEOUSCORERUNTIMEUTILITIES_API UGorgeousSingletonTemplate : public UObject
{
	GENERATED_BODY()

public:

	template<typename T>
	static T* GetSingleton();

	static void DestroySingleton();

protected:

	// Protected constructor to prevent direct instantiation.
	UGorgeousSingletonTemplate() {}
	
private:

	static UGorgeousSingletonTemplate* SingletonInstance;
};