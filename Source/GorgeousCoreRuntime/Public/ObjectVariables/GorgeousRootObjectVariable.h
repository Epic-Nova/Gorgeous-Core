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

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousObjectVariable.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousRootObjectVariable.generated.h"
//<-------------------------------------------------------------------------->

/**
 * The root object variable, serving as the central registry for all object variables.
 *
 * Key features include:
 * - Singleton pattern for global access.
 * - Registry for tracking all object variables.
 * - Hierarchy management for nested variables.
 * - Universal getter and setter functions for dynamic property access.
 * - Cleanup functionality for removing variables from the registry.
 *
 * @note This class provides a centralized and efficient way to manage object variables within the game.
 */
UCLASS(BlueprintType)
class GORGEOUSCORERUNTIME_API UGorgeousRootObjectVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()

public:

    /**
     * Constructor for the root object variable.
     */
    UGorgeousRootObjectVariable();

    /**
     * Gets the singleton instance of the root object variable.
     *
     * @return The root object variable instance.
     */
    UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Root Object Variable")
    static UGorgeousRootObjectVariable* GetRootObjectVariable();

    /**
     * Gets the hierarchy registry of all object variables.
     *
     * @return An array of all registered object variables.
     */
    UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Root Object Variable")
    static TArray<UGorgeousObjectVariable*> GetVariableHierarchyRegistry();

    /**
     * Gets the root variable registry.
     *
     * @return An array of root-level object variables.
     */
    UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Root Object Variable")
    static TArray<UGorgeousObjectVariable*> GetRootVariableRegistry();

    /**
     * Removes a variable from the registry.
     *
     * @param VariableToRemove The variable to remove.
     */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Root Object Variable")
    static void RemoveVariableFromRegistry(UGorgeousObjectVariable* VariableToRemove);

    /**
     * Checks if a given object variable is already registered with the registry
     * 
     * @param Variable The variable to check for existance in the registry
     * @return True if the variable is contained either in the root registry ot the registry of any other variable, false otherwise.
     */
   UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Root Object Variable")
   static bool IsVariableRegistered(UGorgeousObjectVariable* Variable);

    /**
     * Cleans up the registry, optionally performing a full cleanup.
     *
     * @param bFullCleanup Whether to perform a full cleanup.
     */
   UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Root Object Variable")
   static void CleanupRegistry(bool bFullCleanup = false);

    /**
     * Sets the value of a property with any type for an object variable identified by its unique identifier.
     *
     * This function is not intended to be called from C++, it should only be used in Blueprint.
     * For a C++ version of this function, check out the equivalent function in UGorgeousObjectVariable.
     * 
     * @param Identifier The unique identifier of the object variable.
     * @param OptionalPropertyName The name of the property to set.
     * @param Value The value to set.
     *
     * @todo A dropdown that lists the available variables inside OptionalPropertyName with a corresponding TSubclassOf (meta specifier)
     * @todo dynamic pin for the variable type (right click, change to variable type => like the math operations) or automatically set the thunk type with the OptionalPropertyName or default value "Value"
     */
    UFUNCTION(BlueprintCallable, CustomThunk, Category = "Gorgeous Core|Gorgeous Object Variables", meta = (CustomStructureParam = "Value"))
    static void SetUniversalVariable(FGuid Identifier, FName OptionalPropertyName, const int32& Value);

    DECLARE_FUNCTION(execSetUniversalVariable)
    {
       P_GET_STRUCT(FGuid, Identifier);
       P_GET_PROPERTY(FNameProperty, OptionalPropertyName);
       Stack.StepCompiledIn<FProperty>(nullptr);
       const FProperty* SourceProperty  = Stack.MostRecentProperty;
       const void* SourcePropertyAddress  = Stack.MostRecentPropertyAddress;
       P_FINISH;

       if (OptionalPropertyName.IsNone())
       {
          OptionalPropertyName = "Value";
       }

       UGorgeousObjectVariable* FoundObjectVariable = nullptr;

       for (const auto ObjectVariable : GetVariableHierarchyRegistry())
       {
          if (ObjectVariable->UniqueIdentifier == Identifier)
          {
             FoundObjectVariable = ObjectVariable;
             break;
          }
       }

       if (FoundObjectVariable && SourceProperty && SourcePropertyAddress)
       {
          if (const FProperty* TargetProperty = FindFProperty<FProperty>(FoundObjectVariable->GetClass(), OptionalPropertyName))
          {
             if (TargetProperty->SameType(SourceProperty))
             {
                TargetProperty->SetValue_InContainer(FoundObjectVariable, SourcePropertyAddress);
             }
             else
             {
                UGorgeousLoggingBlueprintFunctionLibrary::LogWarningMessage(FString::Printf(TEXT("Property type mismatch for %s"), *OptionalPropertyName.ToString()), "GT.ObjectVariables.Universal.Type_Mismatch", 2.f, Stack.Object);
             }
          }
       }
    }

    /**
     * Gets the value of a property with any type from an object variable identified by its unique identifier.
     *
     * This function is not intended to be called from C++, it should only be used in Blueprint.
     * For a C++ version of this function, check out the equivalent function in UGorgeousObjectVariable.
     * 
     * @param Identifier The unique identifier of the object variable.
     * @param OptionalPropertyName The name of the property to get.
     * @param OutValue The output value.
     *
     * @todo A dropdown that lists the available variables inside OptionalPropertyName with a corresponding TSubclassOf (meta specifier)
     * @todo dynamic pin for the variable type (right click, change to variable type => like the math operations) or automatically set the thunk type with the OptionalPropertyName or default value "Value"
     */
    UFUNCTION(BlueprintPure, CustomThunk, Category = "Gorgeous Core|Gorgeous Object Variables", meta = (CustomStructureParam = "OutValue"))
    static void GetUniversalVariable(FGuid Identifier, FName OptionalPropertyName, int32& OutValue);

    DECLARE_FUNCTION(execGetUniversalVariable)
    {
       P_GET_STRUCT(FGuid, Identifier);
       P_GET_PROPERTY(FNameProperty, OptionalPropertyName);
       Stack.StepCompiledIn<FProperty>(nullptr);
       void* OutValueAddress = Stack.MostRecentPropertyAddress;
       const FProperty* OutValueProperty = Stack.MostRecentProperty;
       P_FINISH;

       if (OptionalPropertyName.IsNone())
       {
          OptionalPropertyName = "Value";
       }

       UGorgeousObjectVariable* FoundObjectVariable = nullptr;

       for (const auto ObjectVariable : GetVariableHierarchyRegistry())
       {
          if (ObjectVariable->UniqueIdentifier == Identifier)
          {
             FoundObjectVariable = ObjectVariable;
             break;
          }
       }

       if (FoundObjectVariable)
       {
          if (const FProperty* SourceProperty = FindFProperty<FProperty>(FoundObjectVariable->GetClass(), OptionalPropertyName))
          {
             if (SourceProperty->SameType(OutValueProperty))
             {
                SourceProperty->CopyCompleteValue(OutValueAddress, SourceProperty->ContainerPtrToValuePtr<void>(FoundObjectVariable));
             }
             else
             {
                UGorgeousLoggingBlueprintFunctionLibrary::LogWarningMessage(FString::Printf(TEXT("Property type mismatch for %s"), *OptionalPropertyName.ToString()), "GT.ObjectVariables.Universal.Type_Mismatch", 2.f, Stack.Object);
             }
          }
       }
    }

    /**
     * Registers a new object variable with the registry.
     *
     * @param NewObjectVariable The object variable to register.
     */
    virtual void RegisterWithRegistry(UGorgeousObjectVariable* NewObjectVariable) override;

protected:

    /**
     * The root variable registry.
     */
    static TArray<TObjectPtr<UGorgeousObjectVariable>> RootVariableRegistry;

    /**
     * The singleton instance of the root object variable.
     */
    static TObjectPtr<UGorgeousRootObjectVariable> SingletonRootInstance;
};