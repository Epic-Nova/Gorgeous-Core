// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "ObjectVariables/GorgeousObjectVariableStructures.h"
#include "GorgeousMultiMapObjectVariablesGetter_I.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class UGorgeousObjectVariable;
//<-------------------------------------------------->

/**
 * Interface for getting multi-map object variables.
 *
 * Key features include:
 * - Functions to get multi-maps of various object variable types.
 * - BlueprintNativeEvent and BlueprintCallable functions for easy use in Blueprints.
 *
 * @note This interface provides a standardized way to access multi-map object variables.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousMultiMapObjectVariablesGetter_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Native interface for getting multi-map object variables.
 */
class GORGEOUSCORERUNTIME_API IGorgeousMultiMapObjectVariablesGetter_I
{
	GENERATED_BODY()

public:

    /**
     * Gets a multi-map of GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A multi-map of GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Object Variable")
    TMap<UGorgeousObjectVariable*, FObjectVariableMultiMapValue> GetObjectVariableMultiMapObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a multi-map of UObjects to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A multi-map of UObjects to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Object")
    TMap<UObject*, FObjectVariableMultiMapValue> GetObjectObjectMultiMapObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a multi-map of UClasses to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A multi-map of UClasses to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Object")
    TMap<UClass*, FObjectVariableMultiMapValue> GetObjectClassMultiMapObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a multi-map of TSoftObjectPtr<UObject> to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A multi-map of TSoftObjectPtr<UObject> to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Object")
    TMap<TSoftObjectPtr<UObject>, FObjectVariableMultiMapValue> GetSoftObjectObjectMultiMapObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a multi-map of TSoftClassPtr<UObject> to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A multi-map of TSoftClassPtr<UObject> to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Object")
    TMap<TSoftClassPtr<UObject>, FObjectVariableMultiMapValue> GetSoftObjectClassMultiMapObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a multi-map of bytes to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A multi-map of bytes to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Byte")
    TMap<uint8, FObjectVariableMultiMapValue> GetByteMultiMapObjectVariable(const FName OptionalVariableName) const;

	/**
	 * Gets a multi-map of floats to GorgeousObjectVariables.
	 *
	 * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
	 * @return A multi-map of floats to GorgeousObjectVariables.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Float")
    TMap<float, FObjectVariableMultiMapValue> GetFloatMultiMapObjectVariable(const FName OptionalVariableName) const;
	
    /**
     * Gets a multi-map of doubles to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A multi-map of doubles to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Double")
    TMap<double, FObjectVariableMultiMapValue> GetDoubleMultiMapObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a multi-map of 64-bit integers to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A multi-map of 64-bit integers to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Integer64")
    TMap<int64, FObjectVariableMultiMapValue> GetInteger64MultiMapObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a multi-map of integers to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A multi-map of integers to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Integer")
    TMap<int32, FObjectVariableMultiMapValue> GetIntegerMultiMapObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a multi-map of FNames to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A multi-map of FNames to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Name")
    TMap<FName, FObjectVariableMultiMapValue> GetNameMultiMapObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a multi-map of FStrings to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @return A multi-map of FStrings to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|String")
    TMap<FString, FObjectVariableMultiMapValue> GetStringMultiMapObjectVariable(const FName OptionalVariableName) const;
};
