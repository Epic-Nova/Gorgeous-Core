// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "UObject/Object.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousSingleton.generated.h"
//<-------------------------------------------------------------------------->

UENUM(BlueprintType)
enum class EGorgeousSingletonScope : uint8
{
    Global,
    World,
    GameInstance,
    Editor
};

/**
 * Used to provide a singleton way of accessing a class.
 *
 * Key features include:
 * - Template-based singleton creation for any UObject class.
 * - Optional scoping by Global, World, GameInstance, or Editor.
 * - Explicit destruction helpers.
 */
UCLASS()
class GORGEOUSCORERUNTIMEUTILITIES_API UGorgeousSingleton : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Gets the singleton instance of the specified type.
     */
    template<typename T>
    static T* GetSingleton(UObject* WorldContextObject = nullptr, EGorgeousSingletonScope Scope = EGorgeousSingletonScope::Global)
    {
        static_assert(TIsDerivedFrom<T, UObject>::IsDerived, "The type provided is not derived from UObject.");
        return Cast<T>(GetOrCreateSingleton(T::StaticClass(), WorldContextObject, Scope));
    }

    /**
     * Destroys the singleton instance for the specified type and scope.
     */
    template<typename T>
    static void DestroySingleton(UObject* WorldContextObject = nullptr, EGorgeousSingletonScope Scope = EGorgeousSingletonScope::Global)
    {
        static_assert(TIsDerivedFrom<T, UObject>::IsDerived, "The type provided is not derived from UObject.");
        DestroySingletonByClass(T::StaticClass(), WorldContextObject, Scope);
    }

    /**
     * Destroys all registered singleton instances.
     */
    static void DestroyAllSingletons();

protected:
    UGorgeousSingleton() {}

private:
    static UObject* GetOrCreateSingleton(UClass* SingletonClass, UObject* WorldContextObject, EGorgeousSingletonScope Scope);
    static void DestroySingletonByClass(UClass* SingletonClass, UObject* WorldContextObject, EGorgeousSingletonScope Scope);
};

/**
 * Attach a singleton accessor to any UObject-derived class.
 */
#define GORGEOUS_ATTACH_SINGLETON(ClassType) \
public: \
    static ClassType* GetSingleton(UObject* WorldContextObject = nullptr, EGorgeousSingletonScope Scope = EGorgeousSingletonScope::Global) \
    { \
        return UGorgeousSingleton::GetSingleton<ClassType>(WorldContextObject, Scope); \
    } \
    static void DestroySingleton(UObject* WorldContextObject = nullptr, EGorgeousSingletonScope Scope = EGorgeousSingletonScope::Global) \
    { \
        UGorgeousSingleton::DestroySingleton<ClassType>(WorldContextObject, Scope); \
    }