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
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousStackObjectVariablesSetter_I.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UGorgeousObjectVariable;
//<------------------------------------------------------------->//<-------------------------------------------------->

/**
 * Interface for setting stack object variables.
 *
 * Key features include:
 * - Functions to set stacks of various object variable types.
 * - BlueprintNativeEvent and BlueprintCallable functions for easy use in Blueprints.
 *
 * @note This interface provides a standardized way to set stack object variables.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousStackObjectVariablesSetter_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Native interface for setting stack object variables.
 */
class GORGEOUSCORERUNTIME_API IGorgeousStackObjectVariablesSetter_I
{
	GENERATED_BODY()

public:

    /**
     * Sets a stack of GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new stack of GorgeousObjectVariables.
     * @return The updated stack of GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Object Variable")
    TArray<UGorgeousObjectVariable*> SetObjectVariableStackObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<UGorgeousObjectVariable*>& NewValue);

    /**
     * Sets a stack of UObjects.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new stack of UObjects.
     * @return The updated stack of UObjects.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Object")
    TArray<UObject*> SetObjectObjectStackObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<UObject*>& NewValue);

    /**
     * Sets a stack of UClasses.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new stack of UClasses.
     * @return The updated stack of UClasses.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Object")
    TArray<UClass*> SetObjectClassStackObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<UClass*>& NewValue);

    /**
     * Sets a stack of TSoftObjectPtr<UObject>.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new stack of TSoftObjectPtr<UObject>.
     * @return The updated stack of TSoftObjectPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Object")
    TArray<TSoftObjectPtr<UObject>> SetSoftObjectObjectStackObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<TSoftObjectPtr<UObject>>& NewValue);

    /**
     * Sets a stack of TSoftClassPtr<UObject>.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new stack of TSoftClassPtr<UObject>.
     * @return The updated stack of TSoftClassPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Object")
    TArray<TSoftClassPtr<UObject>> SetSoftObjectClassStackObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<TSoftClassPtr<UObject>>& NewValue);

    /**
     * Sets a stack of booleans.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new stack of booleans.
     * @return The updated stack of booleans.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Boolean")
    TArray<bool> SetBooleanStackObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<bool>& NewValue);

    /**
     * Sets a stack of bytes.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new stack of bytes.
     * @return The updated stack of bytes.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Byte")
    TArray<uint8> SetByteStackObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<uint8>& NewValue);

	/**
	 * Sets a stack of floats.
	 *
	 * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
	 * @param NewValue The new stack of floats.
	 * @return The updated stack of floats.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Float")
	TArray<float> SetFloatStackObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<float>& NewValue);

    /**
     * Sets a stack of doubles.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new stack of doubles.
     * @return The updated stack of doubles.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Double")
    TArray<double> SetDoubleStackObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<double>& NewValue);

    /**
     * Sets a stack of integers.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new stack of integers.
     * @return The updated stack of integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Integer")
    TArray<int32> SetIntegerStackObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<int32>& NewValue);

    /**
     * Sets a stack of 64-bit integers.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new stack of 64-bit integers.
     * @return The updated stack of 64-bit integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Integer64")
    TArray<int64> SetInteger64StackObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<int64>& NewValue);

    /**
     * Sets a stack of FNames.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new stack of FNames.
     * @return The updated stack of FNames.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Name")
    TArray<FName> SetNameStackObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<FName>& NewValue);

    /**
     * Sets a stack of FRotators.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new stack of FRotators.
     * @return The updated stack of FRotators.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Rotator")
    TArray<FRotator> SetRotatorStackObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<FRotator>& NewValue);

    /**
     * Sets a stack of FStrings.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new stack of FStrings.
     * @return The updated stack of FStrings.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|String")
    TArray<FString> SetStringStackObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<FString>& NewValue);

    /**
     * Sets a stack of FTexts.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new stack of FTexts.
     * @return The updated stack of FTexts.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Stack|Text")
    TArray<FText> SetTextStackObjectVariable(const FName OptionalVariableName, UPARAM(ref) TArray<FText>& NewValue);
};