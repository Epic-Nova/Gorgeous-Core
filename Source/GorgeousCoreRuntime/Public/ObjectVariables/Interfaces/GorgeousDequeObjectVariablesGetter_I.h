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
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousDequeObjectVariablesGetter_I.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class UGorgeousObjectVariable;
//<-------------------------------------------------->

/**
 * Interface for getting deque object variables.
 *
 * Key features include:
 * - Functions to get deques of various object variable types.
 * - BlueprintNativeEvent and BlueprintCallable functions for easy use in Blueprints.
 *
 * @note This interface provides a standardized way to access deque object variables.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousDequeObjectVariablesGetter_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Native interface for getting deque object variables.
 */
class GORGEOUSCORERUNTIME_API IGorgeousDequeObjectVariablesGetter_I
{
	GENERATED_BODY()

public:

    /**
     * Gets a deque of GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A deque of GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Object Variable")
    TArray<UGorgeousObjectVariable*> GetObjectVariableDequeObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a deque of UObjects.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A deque of UObjects.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Object")
    TArray<UObject*> GetObjectObjectDequeObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a deque of UClasses.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A deque of UClasses.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Object")
    TArray<UClass*> GetObjectClassDequeObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a deque of TSoftObjectPtr<UObject>.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A deque of TSoftObjectPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Object")
    TArray<TSoftObjectPtr<UObject>> GetSoftObjectObjectDequeObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a deque of TSoftClassPtr<UObject>.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A deque of TSoftClassPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Object")
    TArray<TSoftClassPtr<UObject>> GetSoftObjectClassDequeObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a deque of booleans.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A deque of booleans.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Boolean")
    TArray<bool> GetBooleanDequeObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a deque of bytes.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A deque of bytes.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Byte")
    TArray<uint8> GetByteDequeObjectVariable(const FName OptionalVariableName) const;

	/**
	 * Gets a deque of floats.
	 *
	 * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
	 * @return A deque of floats.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Float")
	TArray<float> GetFloatDequeObjectVariable(const FName OptionalVariableName) const;
	
    /**
     * Gets a deque of doubles.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A deque of doubles.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Double")
    TArray<double> GetDoubleDequeObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a deque of integers.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A deque of integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Integer")
    TArray<int32> GetIntegerDequeObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a deque of 64-bit integers.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A deque of 64-bit integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Integer64")
    TArray<int64> GetInteger64DequeObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a deque of FNames.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A deque of FNames.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Name")
    TArray<FName> GetNameDequeObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a deque of FRotators.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A deque of FRotators.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Rotator")
    TArray<FRotator> GetRotatorDequeObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a deque of FStrings.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A deque of FStrings.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|String")
    TArray<FString> GetStringDequeObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a deque of FTexts.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A deque of FTexts.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Deque|Text")
    TArray<FText> GetTextDequeObjectVariable(const FName OptionalVariableName) const;
};
