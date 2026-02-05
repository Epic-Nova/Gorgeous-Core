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

//<=====================--- Class Macro Definitions ---======================>

/**
 * Macro to attach singleton functionality to a UObject-derived class.
 * 
 * @param ClassType The class type to which the singleton functionality is being attached.
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
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousSingleton.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Enum defining the scope of the singleton instance.
 * 
 * @author Nils Bergemann
 */
UENUM(BlueprintType)
enum class EGorgeousSingletonScope : uint8
{
    Global,
    World,
    GameInstance,
    Editor
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Singleton
| Functional Name: UGorgeousSingleton
| Parent Class: UObject
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Base class to provide singleton functionality for UObject-derived classes,
| by calling the macro GORGEOUS_ATTACH_SINGLETON in the class definition.
| It allows easy access to a single instance of the class throughout
| the application, with optional scoping to World, GameInstance, or Editor.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS()
class GORGEOUSCORERUNTIMEUTILITIES_API UGorgeousSingleton : public UObject
{
    GENERATED_BODY()

public:
    
    /**
     * Retrieves the singleton instance for the specified type and scope.
     * If the instance does not exist, it will be created.
     * 
     * @param WorldContextObject Optional context object to determine the world scope.
     * @param Scope The scope of the singleton instance (Global, World, GameInstance, Editor).
     * @return The singleton instance of the specified type.
     */
    template<typename T>
    static T* GetSingleton(UObject* WorldContextObject = nullptr, EGorgeousSingletonScope Scope = EGorgeousSingletonScope::Global)
    {
        static_assert(TIsDerivedFrom<T, UObject>::IsDerived, "The type provided is not derived from UObject.");
        return Cast<T>(GetOrCreateSingleton(T::StaticClass(), WorldContextObject, Scope));
    }

    /**
     * Destroys the singleton instance for the specified type and scope.
     * 
     * @param WorldContextObject Optional context object to determine the world scope.
     * @param Scope The scope of the singleton instance (Global, World, GameInstance, Editor).
     */
    template<typename T>
    static void DestroySingleton(UObject* WorldContextObject = nullptr, EGorgeousSingletonScope Scope = EGorgeousSingletonScope::Global)
    {
        static_assert(TIsDerivedFrom<T, UObject>::IsDerived, "The type provided is not derived from UObject.");
        DestroySingletonByClass(T::StaticClass(), WorldContextObject, Scope);
    }
    
    // Destroys all singleton instances across all scopes. 
    static void DestroyAllSingletons();
    
protected:
    
    // Protected constructor to prevent direct instantiation.
    UGorgeousSingleton() {}

private:
    
    /**
     * Internal method to get or create the singleton instance based on class and scope.
     * 
     * @param SingletonClass The class type of the singleton.
     * @param WorldContextObject Optional context object to determine the world scope.
     * @param Scope The scope of the singleton instance (Global, World, GameInstance, Editor).
     * @return The singleton instance as a UObject pointer.
     */
    static UObject* GetOrCreateSingleton(UClass* SingletonClass, UObject* WorldContextObject, EGorgeousSingletonScope Scope);

    /**
     * Internal method to destroy the singleton instance based on class and scope.
     * 
     * @param SingletonClass The class type of the singleton.
     * @param WorldContextObject Optional context object to determine the world scope.
     * @param Scope The scope of the singleton instance (Global, World, GameInstance, Editor).
     */
    static void DestroySingletonByClass(UClass* SingletonClass, UObject* WorldContextObject, EGorgeousSingletonScope Scope);
};