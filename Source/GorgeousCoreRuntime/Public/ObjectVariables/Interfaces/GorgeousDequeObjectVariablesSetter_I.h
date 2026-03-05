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
#pragma once

//<=============================--- Includes ---=============================>
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousDequeObjectVariablesSetter_I.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class UGorgeousObjectVariable;
//<-------------------------------------------------->

/**
 * Interface for setting deque object variables.
 *
 * Key features include:
 * - Functions to set deques of various object variable types.
 * - BlueprintNativeEvent and BlueprintCallable functions for easy use in Blueprints.
 *
 * @note This interface provides a standardized way to set deque object variables.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousDequeObjectVariablesSetter_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Native interface for setting deque object variables.
 */
class GORGEOUSCORERUNTIME_API IGorgeousDequeObjectVariablesSetter_I
{
	GENERATED_BODY()

public:

    /**
     * Sets a deque of GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new deque of GorgeousObjectVariables.
     * @return The updated deque of GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Object Variable")
    TArray<UGorgeousObjectVariable*> SetObjectVariableDequeObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<UGorgeousObjectVariable*>& NewValue);

    /**
     * Sets a deque of UObjects.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new deque of UObjects.
     * @return The updated deque of UObjects.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Object")
    TArray<UObject*> SetObjectObjectDequeObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<UObject*>& NewValue);

    /**
     * Sets a deque of UClasses.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new deque of UClasses.
     * @return The updated deque of UClasses.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Object")
    TArray<UClass*> SetObjectClassDequeObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<UClass*>& NewValue);

    /**
     * Sets a deque of TSoftObjectPtr<UObject>.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new deque of TSoftObjectPtr<UObject>.
     * @return The updated deque of TSoftObjectPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Object")
    TArray<TSoftObjectPtr<UObject>> SetSoftObjectObjectDequeObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<TSoftObjectPtr<UObject>>& NewValue);

    /**
     * Sets a deque of TSoftClassPtr<UObject>.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new deque of TSoftClassPtr<UObject>.
     * @return The updated deque of TSoftClassPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Object")
    TArray<TSoftClassPtr<UObject>> SetSoftObjectClassDequeObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<TSoftClassPtr<UObject>>& NewValue);

    /**
     * Sets a deque of booleans.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new deque of booleans.
     * @return The updated deque of booleans.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Boolean")
    TArray<bool> SetBooleanDequeObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<bool>& NewValue);

    /**
     * Sets a deque of bytes.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new deque of bytes.
     * @return The updated deque of bytes.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Byte")
    TArray<uint8> SetByteDequeObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<uint8>& NewValue);

	/**
	 * Sets a deque of floats.
	 *
	 * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
	 * @param NewValue The new deque of floats.
	 * @return The updated deque of floats.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Float")
	TArray<float> SetFloatDequeObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<float>& NewValue);
	
    /**
     * Sets a deque of doubles.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new deque of doubles.
     * @return The updated deque of doubles.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Double")
    TArray<double> SetDoubleDequeObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<double>& NewValue);

    /**
     * Sets a deque of integers.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new deque of integers.
     * @return The updated deque of integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Integer")
    TArray<int32> SetIntegerDequeObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<int32>& NewValue);

    /**
     * Sets a deque of 64-bit integers.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new deque of 64-bit integers.
     * @return The updated deque of 64-bit integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Integer64")
    TArray<int64> SetInteger64DequeObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<int64>& NewValue);

    /**
     * Sets a deque of FNames.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new deque of FNames.
     * @return The updated deque of FNames.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Name")
    TArray<FName> SetNameDequeObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<FName>& NewValue);

    /**
     * Sets a deque of FRotators.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new deque of FRotators.
     * @return The updated deque of FRotators.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Rotator")
    TArray<FRotator> SetRotatorDequeObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<FRotator>& NewValue);

    /**
     * Sets a deque of FStrings.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new deque of FStrings.
     * @return The updated deque of FStrings.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|String")
    TArray<FString> SetStringDequeObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<FString>& NewValue);

    /**
     * Sets a deque of FTexts.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new deque of FTexts.
     * @return The updated deque of FTexts.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Text")
    TArray<FText> SetTextDequeObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<FText>& NewValue);
};
