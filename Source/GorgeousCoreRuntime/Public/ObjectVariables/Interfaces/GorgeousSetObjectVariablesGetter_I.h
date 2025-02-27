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
//<-------------------------=== Engine Includes ===-------------------------->
#include "UObject/Interface.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousSetObjectVariablesGetter_I.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class UGorgeousObjectVariable;
//<-------------------------------------------------->

/**
 * Interface for getting set object variables.
 *
 * Key features include:
 * - Functions to get sets of various object variable types.
 * - BlueprintNativeEvent and BlueprintCallable functions for easy use in Blueprints.
 *
 * @note This interface provides a standardized way to access set object variables.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousSetObjectVariablesGetter_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Native interface for getting set object variables.
 */
class GORGEOUSCORERUNTIME_API IGorgeousSetObjectVariablesGetter_I
{
	GENERATED_BODY()

public:

    /**
     * Gets a set of GorgeousObjectVariables.
     *
     * @return A set of GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object Variable")
    TSet<UGorgeousObjectVariable*> GetObjectVariableSetObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a set of UObjects.
     *
     * @return A set of UObjects.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object")
    TSet<UObject*> GetObjectObjectSetObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a set of UClasses.
     *
     * @return A set of UClasses.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object")
    TSet<UClass*> GetObjectClassSetObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a set of TSoftObjectPtr<UObject>.
     *
     * @return A set of TSoftObjectPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object")
    TSet<TSoftObjectPtr<UObject>> GetSoftObjectObjectSetObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a set of TSoftClassPtr<UObject>.
     *
     * @return A set of TSoftClassPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object")
    TSet<TSoftClassPtr<UObject>> GetSoftObjectClassSetObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a set of bytes.
     *
     * @return A set of bytes.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Byte")
    TSet<uint8> GetByteSetObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a set of doubles.
     *
     * @return A set of doubles.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Float")
    TSet<double> GetFloatSetObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a set of 64-bit integers.
     *
     * @return A set of 64-bit integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Integer64")
    TSet<int64> GetInteger64SetObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a set of integers.
     *
     * @return A set of integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Integer")
    TSet<int32> GetIntegerSetObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a set of FNames.
     *
     * @return A set of FNames.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Name")
    TSet<FName> GetNameSetObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a set of FStrings.
     *
     * @return A set of FStrings.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|String")
    TSet<FString> GetStringSetObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a set of FTransforms.
     *
     * @return A set of FTransforms.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Transform")
    TSet<FTransform> GetTransformSetObjectVariable(const FName OptionalVariableName) const;

    /**
     * Gets a set of FVectors.
     *
     * @return A set of FVectors.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Vector")
    TSet<FVector> GetVectorSetObjectVariable(const FName OptionalVariableName) const;
};