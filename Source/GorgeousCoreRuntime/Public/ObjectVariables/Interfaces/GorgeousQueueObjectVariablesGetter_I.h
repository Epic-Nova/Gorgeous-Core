// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousQueueObjectVariablesGetter_I.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class UGorgeousObjectVariable;
//<-------------------------------------------------->

/**
 * Interface for getting queue object variables.
 *
 * Key features include:
 * - Functions to get queues of various object variable types.
 * - BlueprintNativeEvent and BlueprintCallable functions for easy use in Blueprints.
 *
 * @note This interface provides a standardized way to access queue object variables.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousQueueObjectVariablesGetter_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Native interface for getting queue object variables.
 */
class GORGEOUSCORERUNTIME_API IGorgeousQueueObjectVariablesGetter_I
{
	GENERATED_BODY()

public:

    /**
     * Gets a queue of GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A queue of GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Object Variable")
    TArray<UGorgeousObjectVariable*> GetObjectVariableQueueObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a queue of UObjects.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A queue of UObjects.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Object")
    TArray<UObject*> GetObjectObjectQueueObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a queue of UClasses.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A queue of UClasses.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Object")
    TArray<UClass*> GetObjectClassQueueObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a queue of TSoftObjectPtr<UObject>.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A queue of TSoftObjectPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Object")
    TArray<TSoftObjectPtr<UObject>> GetSoftObjectObjectQueueObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a queue of TSoftClassPtr<UObject>.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A queue of TSoftClassPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Object")
    TArray<TSoftClassPtr<UObject>> GetSoftObjectClassQueueObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a queue of booleans.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A queue of booleans.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Boolean")
    TArray<bool> GetBooleanQueueObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a queue of bytes.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A queue of bytes.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Byte")
    TArray<uint8> GetByteQueueObjectVariable(const FName OptionalVariableName) const;

	/**
	 * Gets a queue of floats.
	 *
	 * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
	 * @return A queue of floats.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Float")
	TArray<float> GetFloatQueueObjectVariable(const FName OptionalVariableName) const;
	
    /**
     * Gets a queue of doubles.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A queue of doubles.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Double")
    TArray<double> GetDoubleQueueObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a queue of integers.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A queue of integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Integer")
    TArray<int32> GetIntegerQueueObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a queue of 64-bit integers.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A queue of 64-bit integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Integer64")
    TArray<int64> GetInteger64QueueObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a queue of FNames.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A queue of FNames.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Name")
    TArray<FName> GetNameQueueObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a queue of FRotators.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A queue of FRotators.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Rotator")
    TArray<FRotator> GetRotatorQueueObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a queue of FStrings.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A queue of FStrings.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|String")
    TArray<FString> GetStringQueueObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a queue of FTexts.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A queue of FTexts.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Queue|Text")
    TArray<FText> GetTextQueueObjectVariable(const FName OptionalVariableName) const;
};
