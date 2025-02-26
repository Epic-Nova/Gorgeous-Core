// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|      that is has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<-------------------------=== Engine Includes ===-------------------------->
#include "UObject/Interface.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousMapObjectVariablesGetter_I.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class UGorgeousObjectVariable;
//<-------------------------------------------------->

/**
 * Interface for getting map object variables.
 *
 * Key features include:
 * - Functions to get maps of various object variable types.
 * - BlueprintNativeEvent and BlueprintCallable functions for easy use in Blueprints.
 *
 * @note This interface provides a standardized way to access map object variables.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousMapObjectVariablesGetter_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Native interface for getting map object variables.
 */
class GORGEOUSCORERUNTIME_API IGorgeousMapObjectVariablesGetter_I
{
	GENERATED_BODY()

public:

    /**
     * Gets a map of GorgeousObjectVariables.
     *
     * @return A map of GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object Variable")
    TMap<UGorgeousObjectVariable*, UGorgeousObjectVariable*> GetObjectVariableMapObjectVariable() const;

    /**
     * Gets a map of UObjects to GorgeousObjectVariables.
     *
     * @return A map of UObjects to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object")
    TMap<UObject*, UGorgeousObjectVariable*> GetObjectObjectMapObjectVariable() const;

    /**
     * Gets a map of UClasses to GorgeousObjectVariables.
     *
     * @return A map of UClasses to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object")
    TMap<UClass*, UGorgeousObjectVariable*> GetObjectClassMapObjectVariable() const;

    /**
     * Gets a map of TSoftObjectPtr<UObject> to GorgeousObjectVariables.
     *
     * @return A map of TSoftObjectPtr<UObject> to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object")
    TMap<TSoftObjectPtr<UObject>, UGorgeousObjectVariable*> GetSoftObjectObjectMapObjectVariable() const;

    /**
     * Gets a map of TSoftClassPtr<UObject> to GorgeousObjectVariables.
     *
     * @return A map of TSoftClassPtr<UObject> to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object")
    TMap<TSoftClassPtr<UObject>, UGorgeousObjectVariable*> GetSoftObjectClassMapObjectVariable() const;

    /**
     * Gets a map of bytes to GorgeousObjectVariables.
     *
     * @return A map of bytes to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Byte")
    TMap<uint8, UGorgeousObjectVariable*> GetByteMapObjectVariable() const;

    /**
     * Gets a map of doubles to GorgeousObjectVariables.
     *
     * @return A map of doubles to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Float")
    TMap<double, UGorgeousObjectVariable*> GetFloatMapObjectVariable() const;

    /**
     * Gets a map of 64-bit integers to GorgeousObjectVariables.
     *
     * @return A map of 64-bit integers to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Integer64")
    TMap<int64, UGorgeousObjectVariable*> GetInteger64MapObjectVariable() const;

    /**
     * Gets a map of integers to GorgeousObjectVariables.
     *
     * @return A map of integers to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Integer")
    TMap<int32, UGorgeousObjectVariable*> GetIntegerMapObjectVariable() const;

    /**
     * Gets a map of FNames to GorgeousObjectVariables.
     *
     * @return A map of FNames to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Name")
    TMap<FName, UGorgeousObjectVariable*> GetNameMapObjectVariable() const;

    /**
     * Gets a map of FStrings to GorgeousObjectVariables.
     *
     * @return A map of FStrings to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|String")
    TMap<FString, UGorgeousObjectVariable*> GetStringMapObjectVariable() const;

    /**
     * Gets a map of FTransforms to GorgeousObjectVariables.
     *
     * @return A map of FTransforms to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Transform")
    TMap<FTransform, UGorgeousObjectVariable*> GetTransformMapObjectVariable() const;

    /**
     * Gets a map of FVectors to GorgeousObjectVariables.
     *
     * @return A map of FVectors to GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Vector")
    TMap<FVector, UGorgeousObjectVariable*> GetVectorMapObjectVariable() const;
};