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
#include "GorgeousQueueObjectVariablesSetter_I.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class UGorgeousObjectVariable;
//<-------------------------------------------------->

/**
 * Interface for setting queue object variables.
 *
 * Key features include:
 * - Functions to set queues of various object variable types.
 * - BlueprintNativeEvent and BlueprintCallable functions for easy use in Blueprints.
 *
 * @note This interface provides a standardized way to set queue object variables.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousQueueObjectVariablesSetter_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Native interface for setting queue object variables.
 */
class GORGEOUSCORERUNTIME_API IGorgeousQueueObjectVariablesSetter_I
{
	GENERATED_BODY()

public:

    /**
     * Sets a queue of GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new queue of GorgeousObjectVariables.
     * @return The updated queue of GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Object Variable")
    TArray<UGorgeousObjectVariable*> SetObjectVariableQueueObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<UGorgeousObjectVariable*>& NewValue);

    /**
     * Sets a queue of UObjects.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new queue of UObjects.
     * @return The updated queue of UObjects.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Object")
    TArray<UObject*> SetObjectObjectQueueObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<UObject*>& NewValue);

    /**
     * Sets a queue of UClasses.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new queue of UClasses.
     * @return The updated queue of UClasses.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Object")
    TArray<UClass*> SetObjectClassQueueObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<UClass*>& NewValue);

    /**
     * Sets a queue of TSoftObjectPtr<UObject>.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new queue of TSoftObjectPtr<UObject>.
     * @return The updated queue of TSoftObjectPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Object")
    TArray<TSoftObjectPtr<UObject>> SetSoftObjectObjectQueueObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<TSoftObjectPtr<UObject>>& NewValue);

    /**
     * Sets a queue of TSoftClassPtr<UObject>.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new queue of TSoftClassPtr<UObject>.
     * @return The updated queue of TSoftClassPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Object")
    TArray<TSoftClassPtr<UObject>> SetSoftObjectClassQueueObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<TSoftClassPtr<UObject>>& NewValue);

    /**
     * Sets a queue of booleans.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new queue of booleans.
     * @return The updated queue of booleans.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Boolean")
    TArray<bool> SetBooleanQueueObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<bool>& NewValue);

    /**
     * Sets a queue of bytes.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new queue of bytes.
     * @return The updated queue of bytes.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Byte")
    TArray<uint8> SetByteQueueObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<uint8>& NewValue);

	/**
	 * Sets a queue of floats.
	 *
	 * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
	 * @param NewValue The new queue of floats.
	 * @return The updated queue of floats.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Float")
	TArray<float> SetFloatQueueObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<float>& NewValue);
	
    /**
     * Sets a queue of doubles.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new queue of doubles.
     * @return The updated queue of doubles.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Double")
    TArray<double> SetDoubleQueueObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<double>& NewValue);

    /**
     * Sets a queue of integers.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new queue of integers.
     * @return The updated queue of integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Integer")
    TArray<int32> SetIntegerQueueObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<int32>& NewValue);

    /**
     * Sets a queue of 64-bit integers.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new queue of 64-bit integers.
     * @return The updated queue of 64-bit integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Integer64")
    TArray<int64> SetInteger64QueueObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<int64>& NewValue);

    /**
     * Sets a queue of FNames.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new queue of FNames.
     * @return The updated queue of FNames.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Name")
    TArray<FName> SetNameQueueObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<FName>& NewValue);

    /**
     * Sets a queue of FRotators.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new queue of FRotators.
     * @return The updated queue of FRotators.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Rotator")
    TArray<FRotator> SetRotatorQueueObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<FRotator>& NewValue);

    /**
     * Sets a queue of FStrings.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new queue of FStrings.
     * @return The updated queue of FStrings.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|String")
    TArray<FString> SetStringQueueObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<FString>& NewValue);

    /**
     * Sets a queue of FTexts.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new queue of FTexts.
     * @return The updated queue of FTexts.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Text")
    TArray<FText> SetTextQueueObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<FText>& NewValue);
};
