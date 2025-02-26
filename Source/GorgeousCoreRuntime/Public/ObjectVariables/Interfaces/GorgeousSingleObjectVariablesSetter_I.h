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
#include "GorgeousSingleObjectVariablesSetter_I.generated.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
class UGorgeousObjectVariable;
//<-------------------------------------------------->

/**
 * Interface for setting single object variables.
 *
 * Key features include:
 * - Functions to set single object variables of various types.
 * - BlueprintNativeEvent and BlueprintCallable functions for easy use in Blueprints.
 *
 * @note This interface provides a standardized way to set single object variables.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousSingleObjectVariablesSetter_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Native interface for setting single object variables.
 */
class GORGEOUSCORERUNTIME_API IGorgeousSingleObjectVariablesSetter_I
{
	GENERATED_BODY()

public:
	
    /**
     * Sets a GorgeousObjectVariable.
     *
     * @param NewValue The new GorgeousObjectVariable.
     * @return The updated GorgeousObjectVariable.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object Variable")
    UGorgeousObjectVariable* SetObjectVariableSingleObjectVariable(UPARAM(ref) UGorgeousObjectVariable*& NewValue);

    /**
     * Sets a UObject.
     *
     * @param NewValue The new UObject.
     * @return The updated UObject.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
    UObject* SetObjectObjectSingleObjectVariable(UPARAM(ref) UObject*& NewValue);

    /**
     * Sets a UClass.
     *
     * @param NewValue The new UClass.
     * @return The updated UClass.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
    UClass* SetObjectClassSingleObjectVariable(UPARAM(ref) UClass*& NewValue);

    /**
     * Sets a TSoftObjectPtr<UObject>.
     *
     * @param NewValue The new TSoftObjectPtr<UObject>.
     * @return The updated TSoftObjectPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
    TSoftObjectPtr<UObject> SetSoftObjectObjectSingleObjectVariable(UPARAM(ref) TSoftObjectPtr<UObject>& NewValue);

    /**
     * Sets a TSoftClassPtr<UObject>.
     *
     * @param NewValue The new TSoftClassPtr<UObject>.
     * @return The updated TSoftClassPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
    TSoftClassPtr<UObject> SetSoftObjectClassSingleObjectVariable(UPARAM(ref) TSoftClassPtr<UObject>& NewValue);

    /**
     * Sets a boolean.
     *
     * @param NewValue The new boolean.
     * @return The updated boolean.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Boolean")
    bool SetBooleanSingleObjectVariable(UPARAM(ref) bool& NewValue);

    /**
     * Sets a byte.
     *
     * @param NewValue The new byte.
     * @return The updated byte.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Byte")
    uint8 SetByteSingleObjectVariable(UPARAM(ref) uint8& NewValue);

    /**
     * Sets a double.
     *
     * @param NewValue The new double.
     * @return The updated double.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Float")
    double SetFloatSingleObjectVariable(UPARAM(ref) double& NewValue);

    /**
     * Sets a 64-bit integer.
     *
     * @param NewValue The new 64-bit integer.
     * @return The updated 64-bit integer.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Integer64")
    int64 SetInteger64SingleObjectVariable(UPARAM(ref) int64& NewValue);

    /**
     * Sets an integer.
     *
     * @param NewValue The new integer.
     * @return The updated integer.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Integer")
    int32 SetIntegerSingleObjectVariable(UPARAM(ref) int32& NewValue);

    /**
     * Sets an FName.
     *
     * @param NewValue The new FName.
     * @return The updated FName.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Name")
    FName SetNameSingleObjectVariable(UPARAM(ref) FName& NewValue);

    /**
     * Sets an FRotator.
     *
     * @param NewValue The new FRotator.
     * @return The updated FRotator.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Rotator")
    FRotator SetRotatorSingleObjectVariable(UPARAM(ref) FRotator& NewValue);

    /**
     * Sets an FString.
     *
     * @param NewValue The new FString.
     * @return The updated FString.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|String")
    FString SetStringSingleObjectVariable(UPARAM(ref) FString& NewValue);

    /**
     * Sets an FText.
     *
     * @param NewValue The new FText.
     * @return The updated FText.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Text")
    FText SetTextSingleObjectVariable(UPARAM(ref) FText& NewValue);

    /**
     * Sets an FTransform.
     *
     * @param NewValue The new FTransform.
     * @return The updated FTransform.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Transform")
    FTransform SetTransformSingleObjectVariable(UPARAM(ref) FTransform& NewValue);

    /**
     * Sets an FVector.
     *
     * @param NewValue The new FVector.
     * @return The updated FVector.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Vector")
    FVector SetVectorSingleObjectVariable(UPARAM(ref) FVector& NewValue);
};