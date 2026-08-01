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
//<--------------------------=== Module Includes ===------------------------->
#include "ObjectVariables/GorgeousObjectVariableStructures.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousMultiMapObjectVariablesSetter_I.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UGorgeousObjectVariable;
//<------------------------------------------------------------->//<-------------------------------------------------->

/**
 * Interface for setting multi-map object variables.
 *
 * Key features include:
 * - Functions to set multi-maps of various object variable types.
 * - BlueprintNativeEvent and BlueprintCallable functions for easy use in Blueprints.
 *
 * @note This interface provides a standardized way to set multi-map object variables.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousMultiMapObjectVariablesSetter_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Native interface for setting multi-map object variables.
 */
class GORGEOUSCORERUNTIME_API IGorgeousMultiMapObjectVariablesSetter_I
{
	GENERATED_BODY()

public:

    /**
     * Sets a multi-map of GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new multi-map of GorgeousObjectVariables.
     * @return The updated multi-map of GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Object Variable")
    TMap<UGorgeousObjectVariable*, FObjectVariableMultiMapValue> SetObjectVariableMultiMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<UGorgeousObjectVariable*, FObjectVariableMultiMapValue>& NewValue);

    /**
     * Sets a multi-map of UObjects to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new multi-map of UObjects to GorgeousObjectVariables.
     * @return The updated multi-map of UObjects to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Object")
    TMap<UObject*, FObjectVariableMultiMapValue> SetObjectObjectMultiMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<UObject*, FObjectVariableMultiMapValue>& NewValue);

    /**
     * Sets a multi-map of UClasses to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new multi-map of UClasses to GorgeousObjectVariables.
     * @return The updated multi-map of UClasses to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Object")
    TMap<UClass*, FObjectVariableMultiMapValue> SetObjectClassMultiMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<UClass*, FObjectVariableMultiMapValue>& NewValue);

    /**
     * Sets a multi-map of TSoftObjectPtr<UObject> to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new multi-map of TSoftObjectPtr<UObject> to GorgeousObjectVariables.
     * @return The updated multi-map of TSoftObjectPtr<UObject> to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Object")
    TMap<TSoftObjectPtr<UObject>, FObjectVariableMultiMapValue> SetSoftObjectObjectMultiMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<TSoftObjectPtr<UObject>, FObjectVariableMultiMapValue>& NewValue);

    /**
     * Sets a multi-map of TSoftClassPtr<UObject> to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new multi-map of TSoftClassPtr<UObject> to GorgeousObjectVariables.
     * @return The updated multi-map of TSoftClassPtr<UObject> to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Object")
    TMap<TSoftClassPtr<UObject>, FObjectVariableMultiMapValue> SetSoftObjectClassMultiMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<TSoftClassPtr<UObject>, FObjectVariableMultiMapValue>& NewValue);

    /**
     * Sets a multi-map of bytes to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new multi-map of bytes to GorgeousObjectVariables.
     * @return The updated multi-map of bytes to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Byte")
    TMap<uint8, FObjectVariableMultiMapValue> SetByteMultiMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<uint8, FObjectVariableMultiMapValue>& NewValue);

	/**
	 * Sets a multi-map of floats to GorgeousObjectVariables.
	 *
	 * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
	 * @param NewValue The new multi-map of floats to GorgeousObjectVariables.
	 * @return The updated multi-map of floats to GorgeousObjectVariables.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Float")
    TMap<float, FObjectVariableMultiMapValue> SetFloatMultiMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<float, FObjectVariableMultiMapValue>& NewValue);

    /**
     * Sets a multi-map of doubles to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new multi-map of doubles to GorgeousObjectVariables.
     * @return The updated multi-map of doubles to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Double")
    TMap<double, FObjectVariableMultiMapValue> SetDoubleMultiMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<double, FObjectVariableMultiMapValue>& NewValue);

    /**
     * Sets a multi-map of 64-bit integers to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new multi-map of 64-bit integers to GorgeousObjectVariables.
     * @return The updated multi-map of 64-bit integers to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Integer64")
    TMap<int64, FObjectVariableMultiMapValue> SetInteger64MultiMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<int64, FObjectVariableMultiMapValue>& NewValue);

    /**
     * Sets a multi-map of integers to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new multi-map of integers to GorgeousObjectVariables.
     * @return The updated multi-map of integers to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Integer")
    TMap<int32, FObjectVariableMultiMapValue> SetIntegerMultiMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<int32, FObjectVariableMultiMapValue>& NewValue);

    /**
     * Sets a multi-map of FNames to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new multi-map of FNames to GorgeousObjectVariables.
     * @return The updated multi-map of FNames to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|Name")
    TMap<FName, FObjectVariableMultiMapValue> SetNameMultiMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<FName, FObjectVariableMultiMapValue>& NewValue);

    /**
     * Sets a multi-map of FStrings to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new multi-map of FStrings to GorgeousObjectVariables.
     * @return The updated multi-map of FStrings to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Multi Map|String")
    TMap<FString, FObjectVariableMultiMapValue> SetStringMultiMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<FString, FObjectVariableMultiMapValue>& NewValue);
};