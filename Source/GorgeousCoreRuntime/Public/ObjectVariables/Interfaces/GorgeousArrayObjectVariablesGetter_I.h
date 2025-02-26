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
#include "GorgeousArrayObjectVariablesGetter_I.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class UGorgeousObjectVariable;
//<-------------------------------------------------->

/**
 * Interface for getting array object variables.
 *
 * Key features include:
 * - Functions to get arrays of various object variable types.
 * - BlueprintNativeEvent and BlueprintCallable functions for easy use in Blueprints.
 *
 * @note This interface provides a standardized way to access array object variables.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousArrayObjectVariablesGetter_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Native interface for getting array object variables.
 */
class GORGEOUSCORERUNTIME_API IGorgeousArrayObjectVariablesGetter_I
{
	GENERATED_BODY()

public:

    /**
     * Gets an array of GorgeousObjectVariables.
     *
     * @return An array of GorgeousObjectVariables.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object Variable")
    TArray<UGorgeousObjectVariable*> GetObjectVariableArrayObjectVariable() const;

    /**
     * Gets an array of UObjects.
     *
     * @return An array of UObjects.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object")
    TArray<UObject*> GetObjectObjectArrayObjectVariable() const;

    /**
     * Gets an array of UClasses.
     *
     * @return An array of UClasses.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object")
    TArray<UClass*> GetObjectClassArrayObjectVariable() const;

    /**
     * Gets an array of TSoftObjectPtr<UObject>.
     *
     * @return An array of TSoftObjectPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object")
    TArray<TSoftObjectPtr<UObject>> GetSoftObjectObjectArrayObjectVariable() const;

    /**
     * Gets an array of TSoftClassPtr<UObject>.
     *
     * @return An array of TSoftClassPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object")
    TArray<TSoftClassPtr<UObject>> GetSoftObjectClassArrayObjectVariable() const;

    /**
     * Gets an array of booleans.
     *
     * @return An array of booleans.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Boolean")
    TArray<bool> GetBooleanArrayObjectVariable() const;

    /**
     * Gets an array of bytes.
     *
     * @return An array of bytes.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Byte")
    TArray<uint8> GetByteArrayObjectVariable() const;

    /**
     * Gets an array of doubles.
     *
     * @return An array of doubles.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Float")
    TArray<double> GetFloatArrayObjectVariable() const;

    /**
     * Gets an array of integers.
     *
     * @return An array of integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Integer")
    TArray<int32> GetIntegerArrayObjectVariable() const;

    /**
     * Gets an array of 64-bit integers.
     *
     * @return An array of 64-bit integers.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Integer64")
    TArray<int64> GetInteger64ArrayObjectVariable() const;

    /**
     * Gets an array of FNames.
     *
     * @return An array of FNames.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Name")
    TArray<FName> GetNameArrayObjectVariable() const;

    /**
     * Gets an array of FRotators.
     *
     * @return An array of FRotators.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Rotator")
    TArray<FRotator> GetRotatorArrayObjectVariable() const;

    /**
     * Gets an array of FStrings.
     *
     * @return An array of FStrings.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|String")
    TArray<FString> GetStringArrayObjectVariable() const;

    /**
     * Gets an array of FTexts.
     *
     * @return An array of FTexts.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Text")
    TArray<FText> GetTextArrayObjectVariable() const;

    /**
     * Gets an array of FTransforms.
     *
     * @return An array of FTransforms.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Transform")
    TArray<FTransform> GetTransformArrayObjectVariable() const;

    /**
     * Gets an array of FVectors.
     *
     * @return An array of FVectors.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Vector")
    TArray<FVector> GetVectorArrayObjectVariable() const;
};