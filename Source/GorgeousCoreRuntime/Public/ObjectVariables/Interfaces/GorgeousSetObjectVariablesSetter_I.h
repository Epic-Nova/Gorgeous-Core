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
//<-------------------------=== Engine Includes ===-------------------------->
#include "UObject/Interface.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousSetObjectVariablesSetter_I.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class UGorgeousObjectVariable;
//<-------------------------------------------------->

/**
 * Interface for setting set object variables.
 *
 * Key features include:
 * - Functions to set sets of various object variable types.
 * - BlueprintNativeEvent and BlueprintCallable functions for easy use in Blueprints.
 *
 * @note This interface provides a standardized way to set set object variables.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousSetObjectVariablesSetter_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Native interface for setting set object variables.
 */
class GORGEOUSCORERUNTIME_API IGorgeousSetObjectVariablesSetter_I
{
	GENERATED_BODY()

public:

    /**
     * Sets a set of GorgeousObjectVariables.
     *
     * @param NewValue The new set of GorgeousObjectVariables.
     * @return The updated set of GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object Variable")
    TSet<UGorgeousObjectVariable*> SetObjectVariableSetObjectVariable(const FName OptionalVariableName, UPARAM(ref) TSet<UGorgeousObjectVariable*>& NewValue);

    /**
     * Sets a set of UObjects.
     *
     * @param NewValue The new set of UObjects.
     * @return The updated set of UObjects.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object")
    TSet<UObject*> SetObjectObjectSetObjectVariable(const FName OptionalVariableName, UPARAM(ref) TSet<UObject*>& NewValue);

    /**
     * Sets a set of UClasses.
     *
     * @param NewValue The new set of UClasses.
     * @return The updated set of UClasses.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object")
    TSet<UClass*> SetObjectClassSetObjectVariable(const FName OptionalVariableName, UPARAM(ref) TSet<UClass*>& NewValue);

    /**
     * Sets a set of TSoftObjectPtr<UObject>.
     *
     * @param NewValue The new set of TSoftObjectPtr<UObject>.
     * @return The updated set of TSoftObjectPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object")
    TSet<TSoftObjectPtr<UObject>> SetSoftObjectObjectSetObjectVariable(const FName OptionalVariableName, UPARAM(ref) TSet<TSoftObjectPtr<UObject>>& NewValue);

    /**
     * Sets a set of TSoftClassPtr<UObject>.
     *
     * @param NewValue The new set of TSoftClassPtr<UObject>.
     * @return The updated set of TSoftClassPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object")
    TSet<TSoftClassPtr<UObject>> SetSoftObjectClassSetObjectVariable(const FName OptionalVariableName, UPARAM(ref) TSet<TSoftClassPtr<UObject>>& NewValue);

    /**
     * Sets a set of bytes.
     *
     * @param NewValue The new set of bytes.
     * @return The updated set of bytes.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Byte")
    TSet<uint8> SetByteSetObjectVariable(const FName OptionalVariableName, UPARAM(ref) TSet<uint8>& NewValue);

    /**
     * Sets a set of doubles.
     *
     * @param NewValue The new set of doubles.
     * @return The updated set of doubles.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Float")
    TSet<double> SetFloatSetObjectVariable(const FName OptionalVariableName, UPARAM(ref) TSet<double>& NewValue);

    /**
     * Sets a set of 64-bit integers.
     *
     * @param NewValue The new set of 64-bit integers.
     * @return The updated set of 64-bit integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Integer64")
    TSet<int64> SetInteger64SetObjectVariable(const FName OptionalVariableName, UPARAM(ref) TSet<int64>& NewValue);

    /**
     * Sets a set of integers.
     *
     * @param NewValue The new set of integers.
     * @return The updated set of integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Integer")
    TSet<int32> SetIntegerSetObjectVariable(const FName OptionalVariableName, UPARAM(ref) TSet<int32>& NewValue);

    /**
     * Sets a set of FNames.
     *
     * @param NewValue The new set of FNames.
     * @return The updated set of FNames.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Name")
    TSet<FName> SetNameSetObjectVariable(const FName OptionalVariableName, UPARAM(ref) TSet<FName>& NewValue);

    /**
     * Sets a set of FStrings.
     *
     * @param NewValue The new set of FStrings.
     * @return The updated set of FStrings.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|String")
    TSet<FString> SetStringSetObjectVariable(const FName OptionalVariableName, UPARAM(ref) TSet<FString>& NewValue);

    /**
     * Sets a set of FTransforms.
     *
     * @param NewValue The new set of FTransforms.
     * @return The updated set of FTransforms.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Transform")
    TSet<FTransform> SetTransformSetObjectVariable(const FName OptionalVariableName, UPARAM(ref) TSet<FTransform>& NewValue);

    /**
     * Sets a set of FVectors.
     *
     * @param NewValue The new set of FVectors.
     * @return The updated set of FVectors.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Vector")
    TSet<FVector> SetVectorSetObjectVariable(const FName OptionalVariableName, UPARAM(ref) TSet<FVector>& NewValue);
};