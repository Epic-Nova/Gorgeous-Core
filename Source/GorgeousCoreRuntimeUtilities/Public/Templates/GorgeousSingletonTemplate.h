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

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<-------------------------=== Engine Includes ===-------------------------->
#include "UObject/Object.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousSingletonTemplate.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Used to provide a singleton way of accessing a class that depends on this template.
 *
 * Key features include:
 * - Template-based singleton creation.
 * - Singleton destruction.
 * - Protected constructor to prevent direct instantiation.
 *
 * @note This class provides a generic singleton pattern for UObjects.
 */
UCLASS()
class GORGEOUSCORERUNTIMEUTILITIES_API UGorgeousSingletonTemplate : public UObject
{
    GENERATED_BODY()

public:

    /**
     * Gets the singleton instance of the specified type.
     *
     * @tparam T The type of the singleton.
     * @return The singleton instance.
     */
    template<typename T>
    static T* GetSingleton();

    /**
     * Destroys the singleton instance.
     */
    static void DestroySingleton();

protected:

    /**
     * Protected constructor to prevent direct instantiation.
     */
    UGorgeousSingletonTemplate() {}

private:

    /**
     * The singleton instance.
     */
    static UGorgeousSingletonTemplate* SingletonInstance;
};