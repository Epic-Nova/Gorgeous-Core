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
#include "GorgeousSingleObjectVariablesGetter_I.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class UGorgeousObjectVariable;
//<-------------------------------------------------->

/**
 * Interface for getting single object variables.
 *
 * Key features include:
 * - Functions to get single object variables of various types.
 * - BlueprintNativeEvent and BlueprintCallable functions for easy use in Blueprints.
 *
 * @note This interface provides a standardized way to access single object variables.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousSingleObjectVariablesGetter_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Native interface for getting single object variables.
 */
class GORGEOUSCORERUNTIME_API IGorgeousSingleObjectVariablesGetter_I
{
	GENERATED_BODY()

public:

    /**
     * Gets a GorgeousObjectVariable.
     *
     * @return A GorgeousObjectVariable.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object Variable")
    UGorgeousObjectVariable* GetObjectVariableSingleObjectVariable() const;

    /**
     * Gets a UObject.
     *
     * @return A UObject.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
    UObject* GetObjectObjectSingleObjectVariable() const;

    /**
     * Gets a UClass.
     *
     * @return A UClass.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
    UClass* GetObjectClassSingleObjectVariable() const;

    /**
     * Gets a TSoftObjectPtr<UObject>.
     *
     * @return A TSoftObjectPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
    TSoftObjectPtr<UObject> GetSoftObjectObjectSingleObjectVariable() const;

    /**
     * Gets a TSoftClassPtr<UObject>.
     *
     * @return A TSoftClassPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
    TSoftClassPtr<UObject> GetSoftObjectClassSingleObjectVariable() const;

    /**
     * Gets a boolean.
     *
     * @return A boolean.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Boolean")
    bool GetBooleanSingleObjectVariable() const;

    /**
     * Gets a byte.
     *
     * @return A byte.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Byte")
    uint8 GetByteSingleObjectVariable() const;

    /**
     * Gets a double.
     *
     * @return A double.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Float")
    double GetFloatSingleObjectVariable() const;

    /**
     * Gets a 64-bit integer.
     *
     * @return A 64-bit integer.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Integer64")
    int64 GetInteger64SingleObjectVariable() const;

    /**
     * Gets an integer.
     *
     * @return An integer.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Integer")
    int32 GetIntegerSingleObjectVariable() const;

    /**
     * Gets an FName.
     *
     * @return An FName.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Name")
    FName GetNameSingleObjectVariable() const;

    /**
     * Gets an FRotator.
     *
     * @return An FRotator.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Rotator")
    FRotator GetRotatorSingleObjectVariable() const;

    /**
     * Gets an FString.
     *
     * @return An FString.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|String")
    FString GetStringSingleObjectVariable() const;

    /**
     * Gets an FText.
     *
     * @return An FText.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Text")
    FText GetTextSingleObjectVariable() const;

    /**
     * Gets an FTransform.
     *
     * @return An FTransform.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Transform")
    FTransform GetTransformSingleObjectVariable() const;

    /**
     * Gets an FVector.
     *
     * @return An FVector.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Vector")
    FVector GetVectorSingleObjectVariable() const;
};