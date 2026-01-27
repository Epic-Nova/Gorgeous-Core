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
#include "GorgeousStackObjectVariablesGetter_I.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class UGorgeousObjectVariable;
//<-------------------------------------------------->

/**
 * Interface for getting stack object variables.
 *
 * Key features include:
 * - Functions to get stacks of various object variable types.
 * - BlueprintNativeEvent and BlueprintCallable functions for easy use in Blueprints.
 *
 * @note This interface provides a standardized way to access stack object variables.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousStackObjectVariablesGetter_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Native interface for getting stack object variables.
 */
class GORGEOUSCORERUNTIME_API IGorgeousStackObjectVariablesGetter_I
{
	GENERATED_BODY()

public:

    /**
     * Gets a stack of GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A stack of GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Object Variable")
    TArray<UGorgeousObjectVariable*> GetObjectVariableStackObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a stack of UObjects.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A stack of UObjects.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Object")
    TArray<UObject*> GetObjectObjectStackObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a stack of UClasses.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A stack of UClasses.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Object")
    TArray<UClass*> GetObjectClassStackObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a stack of TSoftObjectPtr<UObject>.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A stack of TSoftObjectPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Object")
    TArray<TSoftObjectPtr<UObject>> GetSoftObjectObjectStackObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a stack of TSoftClassPtr<UObject>.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A stack of TSoftClassPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Object")
    TArray<TSoftClassPtr<UObject>> GetSoftObjectClassStackObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a stack of booleans.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A stack of booleans.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Boolean")
    TArray<bool> GetBooleanStackObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a stack of bytes.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A stack of bytes.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Byte")
    TArray<uint8> GetByteStackObjectVariable(const FName OptionalVariableName) const;

	/**
	 * Gets a stack of floats.
	 *
	 * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
	 * @return A stack of floats.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Float")
	TArray<float> GetFloatStackObjectVariable(const FName OptionalVariableName) const;
	
    /**
     * Gets a stack of doubles.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A stack of doubles.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Double")
    TArray<double> GetDoubleStackObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a stack of integers.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A stack of integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Integer")
    TArray<int32> GetIntegerStackObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a stack of 64-bit integers.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A stack of 64-bit integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Integer64")
    TArray<int64> GetInteger64StackObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a stack of FNames.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A stack of FNames.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Name")
    TArray<FName> GetNameStackObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a stack of FRotators.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A stack of FRotators.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Rotator")
    TArray<FRotator> GetRotatorStackObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a stack of FStrings.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A stack of FStrings.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|String")
    TArray<FString> GetStringStackObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a stack of FTexts.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A stack of FTexts.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Text")
    TArray<FText> GetTextStackObjectVariable(const FName OptionalVariableName) const;
};
