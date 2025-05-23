﻿// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
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
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousArrayObjectVariablesSetter_I.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class UGorgeousObjectVariable;
//<-------------------------------------------------->

/**
 * Interface for setting array object variables.
 *
 * Key features include:
 * - Functions to set arrays of various object variable types.
 * - BlueprintNativeEvent and BlueprintCallable functions for easy use in Blueprints.
 *
 * @note This interface provides a standardized way to set array object variables.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousArrayObjectVariablesSetter_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Native interface for setting array object variables.
 */
class GORGEOUSCORERUNTIME_API IGorgeousArrayObjectVariablesSetter_I
{
	GENERATED_BODY()

public:

    /**
     * Sets an array of GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new array of GorgeousObjectVariables.
     * @return The updated array of GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object Variable")
    TArray<UGorgeousObjectVariable*> SetObjectVariableArrayObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<UGorgeousObjectVariable*>& NewValue);

    /**
     * Sets an array of UObjects.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new array of UObjects.
     * @return The updated array of UObjects.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object")
    TArray<UObject*> SetObjectObjectArrayObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<UObject*>& NewValue);

    /**
     * Sets an array of UClasses.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new array of UClasses.
     * @return The updated array of UClasses.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object")
    TArray<UClass*> SetObjectClassArrayObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<UClass*>& NewValue);

    /**
     * Sets an array of TSoftObjectPtr<UObject>.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new array of TSoftObjectPtr<UObject>.
     * @return The updated array of TSoftObjectPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object")
    TArray<TSoftObjectPtr<UObject>> SetSoftObjectObjectArrayObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<TSoftObjectPtr<UObject>>& NewValue);

    /**
     * Sets an array of TSoftClassPtr<UObject>.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new array of TSoftClassPtr<UObject>.
     * @return The updated array of TSoftClassPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object")
    TArray<TSoftClassPtr<UObject>> SetSoftObjectClassArrayObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<TSoftClassPtr<UObject>>& NewValue);

    /**
     * Sets an array of booleans.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new array of booleans.
     * @return The updated array of booleans.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Boolean")
    TArray<bool> SetBooleanArrayObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<bool>& NewValue);

    /**
     * Sets an array of bytes.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new array of bytes.
     * @return The updated array of bytes.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Byte")
    TArray<uint8> SetByteArrayObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<uint8>& NewValue);

    /**
     * Sets an array of doubles.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new array of doubles.
     * @return The updated array of doubles.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Float")
    TArray<double> SetFloatArrayObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<double>& NewValue);

    /**
     * Sets an array of integers.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new array of integers.
     * @return The updated array of integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Integer")
    TArray<int32> SetIntegerArrayObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<int32>& NewValue);

    /**
     * Sets an array of 64-bit integers.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new array of 64-bit integers.
     * @return The updated array of 64-bit integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Integer64")
    TArray<int64> SetInteger64ArrayObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<int64>& NewValue);

    /**
     * Sets an array of FNames.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new array of FNames.
     * @return The updated array of FNames.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Name")
    TArray<FName> SetNameArrayObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<FName>& NewValue);

    /**
     * Sets an array of FRotators.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new array of FRotators.
     * @return The updated array of FRotators.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Rotator")
    TArray<FRotator> SetRotatorArrayObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<FRotator>& NewValue);

    /**
     * Sets an array of FStrings.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new array of FStrings.
     * @return The updated array of FStrings.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|String")
    TArray<FString> SetStringArrayObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<FString>& NewValue);

    /**
     * Sets an array of FTexts.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new array of FTexts.
     * @return The updated array of FTexts.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Text")
    TArray<FText> SetTextArrayObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<FText>& NewValue);

    /**
     * Sets an array of FTransforms.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new array of FTransforms.
     * @return The updated array of FTransforms.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Transform")
    TArray<FTransform> SetTransformArrayObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<FTransform>& NewValue);

    /**
     * Sets an array of FVectors.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new array of FVectors.
     * @return The updated array of FVectors.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Vector")
	TArray<FVector> SetVectorArrayObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<FVector>& NewValue);
};