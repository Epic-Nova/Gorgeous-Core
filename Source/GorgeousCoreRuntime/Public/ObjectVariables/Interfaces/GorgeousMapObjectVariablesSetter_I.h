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
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousMapObjectVariablesSetter_I.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class UGorgeousObjectVariable;
//<-------------------------------------------------->

/**
 * Interface for setting map object variables.
 *
 * Key features include:
 * - Functions to set maps of various object variable types.
 * - BlueprintNativeEvent and BlueprintCallable functions for easy use in Blueprints.
 *
 * @note This interface provides a standardized way to set map object variables.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousMapObjectVariablesSetter_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Native interface for setting map object variables.
 */
class GORGEOUSCORERUNTIME_API IGorgeousMapObjectVariablesSetter_I
{
	GENERATED_BODY()

public:

    /**
     * Sets a map of GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new map of GorgeousObjectVariables.
     * @return The updated map of GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object Variable")
    TMap<UGorgeousObjectVariable*, UGorgeousObjectVariable*> SetObjectVariableMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<UGorgeousObjectVariable*, UGorgeousObjectVariable*>& NewValue);

    /**
     * Sets a map of UObjects to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new map of UObjects to GorgeousObjectVariables.
     * @return The updated map of UObjects to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object")
    TMap<UObject*, UGorgeousObjectVariable*> SetObjectObjectMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<UObject*, UGorgeousObjectVariable*>& NewValue);

    /**
    * Sets a map of UClasses to GorgeousObjectVariables.
    *
    * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
    * @param NewValue The new map of UClasses to GorgeousObjectVariables.
    * @return The updated map of UClasses to GorgeousObjectVariables.
    */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object")
    TMap<UClass*, UGorgeousObjectVariable*> SetObjectClassMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<UClass*, UGorgeousObjectVariable*>& NewValue);

    /**
     * Sets a map of TSoftObjectPtr<UObject> to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new map of TSoftObjectPtr<UObject> to GorgeousObjectVariables.
     * @return The updated map of TSoftObjectPtr<UObject> to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object")
    TMap<TSoftObjectPtr<UObject>, UGorgeousObjectVariable*> SetSoftObjectObjectMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<TSoftObjectPtr<UObject>, UGorgeousObjectVariable*>& NewValue);

    /**
     * Sets a map of TSoftClassPtr<UObject> to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new map of TSoftClassPtr<UObject> to GorgeousObjectVariables.
     * @return The updated map of TSoftClassPtr<UObject> to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object")
    TMap<TSoftClassPtr<UObject>, UGorgeousObjectVariable*> SetSoftObjectClassMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<TSoftClassPtr<UObject>, UGorgeousObjectVariable*>& NewValue);

    /**
     * Sets a map of bytes to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new map of bytes to GorgeousObjectVariables.
     * @return The updated map of bytes to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Byte")
    TMap<uint8, UGorgeousObjectVariable*> SetByteMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<uint8, UGorgeousObjectVariable*>& NewValue);

    /**
     * Sets a map of doubles to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new map of doubles to GorgeousObjectVariables.
     * @return The updated map of doubles to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Float")
    TMap<double, UGorgeousObjectVariable*> SetFloatMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<double, UGorgeousObjectVariable*>& NewValue);

    /**
     * Sets a map of 64-bit integers to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new map of 64-bit integers to GorgeousObjectVariables.
     * @return The updated map of 64-bit integers to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Integer64")
    TMap<int64, UGorgeousObjectVariable*> SetInteger64MapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<int64, UGorgeousObjectVariable*>& NewValue);

    /**
     * Sets a map of integers to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new map of integers to GorgeousObjectVariables.
     * @return The updated map of integers to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Integer")
    TMap<int32, UGorgeousObjectVariable*> SetIntegerMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<int32, UGorgeousObjectVariable*>& NewValue);

    /**
     * Sets a map of FNames to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new map of FNames to GorgeousObjectVariables.
     * @return The updated map of FNames to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Name")
    TMap<FName, UGorgeousObjectVariable*> SetNameMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<FName, UGorgeousObjectVariable*>& NewValue);

    /**
     * Sets a map of FStrings to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new map of FStrings to GorgeousObjectVariables.
     * @return The updated map of FStrings to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|String")
    TMap<FString, UGorgeousObjectVariable*> SetStringMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<FString, UGorgeousObjectVariable*>& NewValue);

    /**
     * Sets a map of FTransforms to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new map of FTransforms to GorgeousObjectVariables.
     * @return The updated map of FTransforms to GorgeousObjectVariables.
     */
    UFUNCTION(NotBlueprintable, BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Transform")
    TMap<FTransform, UGorgeousObjectVariable*> SetTransformMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<FTransform, UGorgeousObjectVariable*>& NewValue);

    /**
     * Sets a map of FVectors to GorgeousObjectVariables.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new map of FVectors to GorgeousObjectVariables.
     * @return The updated map of FVectors to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Vector")
	TMap<FVector, UGorgeousObjectVariable*> SetVectorMapObjectVariable(const FName OptionalVariableName, UPARAM(ref) TMap<FVector, UGorgeousObjectVariable*>& NewValue);
};