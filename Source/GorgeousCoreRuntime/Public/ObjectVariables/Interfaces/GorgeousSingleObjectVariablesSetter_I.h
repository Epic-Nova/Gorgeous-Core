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
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new GorgeousObjectVariable.
     * @return The updated GorgeousObjectVariable.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object Variable")
    UGorgeousObjectVariable* SetObjectVariableSingleObjectVariable(const FName OptionalVariableName, UPARAM(ref) UGorgeousObjectVariable*& NewValue);

    /**
     * Sets a UObject.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new UObject.
     * @return The updated UObject.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
    UObject* SetObjectObjectSingleObjectVariable(const FName OptionalVariableName, UPARAM(ref) UObject*& NewValue);

    /**
     * Sets a UClass.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new UClass.
     * @return The updated UClass.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
    UClass* SetObjectClassSingleObjectVariable(const FName OptionalVariableName, UPARAM(ref) UClass*& NewValue);

    /**
     * Sets a TSoftObjectPtr<UObject>.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new TSoftObjectPtr<UObject>.
     * @return The updated TSoftObjectPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
    TSoftObjectPtr<UObject> SetSoftObjectObjectSingleObjectVariable(const FName OptionalVariableName, UPARAM(ref) TSoftObjectPtr<UObject>& NewValue);

    /**
     * Sets a TSoftClassPtr<UObject>.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new TSoftClassPtr<UObject>.
     * @return The updated TSoftClassPtr<UObject>.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
    TSoftClassPtr<UObject> SetSoftObjectClassSingleObjectVariable(const FName OptionalVariableName, UPARAM(ref) TSoftClassPtr<UObject>& NewValue);

    /**
     * Sets a boolean.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new boolean.
     * @return The updated boolean.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Boolean")
    bool SetBooleanSingleObjectVariable(const FName OptionalVariableName, UPARAM(ref) bool& NewValue);

    /**
     * Sets a byte.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new byte.
     * @return The updated byte.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Byte")
    uint8 SetByteSingleObjectVariable(const FName OptionalVariableName, UPARAM(ref) uint8& NewValue);

    /**
     * Sets a double.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new double.
     * @return The updated double.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Float")
    double SetFloatSingleObjectVariable(const FName OptionalVariableName, UPARAM(ref) double& NewValue);

    /**
     * Sets a 64-bit integer.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new 64-bit integer.
     * @return The updated 64-bit integer.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Integer64")
    int64 SetInteger64SingleObjectVariable(const FName OptionalVariableName, UPARAM(ref) int64& NewValue);

    /**
     * Sets an integer.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new integer.
     * @return The updated integer.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Integer")
    int32 SetIntegerSingleObjectVariable(const FName OptionalVariableName, UPARAM(ref) int32& NewValue);

    /**
     * Sets an FName.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new FName.
     * @return The updated FName.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Name")
    FName SetNameSingleObjectVariable(const FName OptionalVariableName, UPARAM(ref) FName& NewValue);

    /**
     * Sets an FRotator.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new FRotator.
     * @return The updated FRotator.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Rotator")
    FRotator SetRotatorSingleObjectVariable(const FName OptionalVariableName, UPARAM(ref) FRotator& NewValue);

    /**
     * Sets an FString.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new FString.
     * @return The updated FString.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|String")
    FString SetStringSingleObjectVariable(const FName OptionalVariableName, UPARAM(ref) FString& NewValue);

    /**
     * Sets an FText.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new FText.
     * @return The updated FText.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Text")
    FText SetTextSingleObjectVariable(const FName OptionalVariableName, UPARAM(ref) FText& NewValue);

    /**
     * Sets an FTransform.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new FTransform.
     * @return The updated FTransform.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Transform")
    FTransform SetTransformSingleObjectVariable(const FName OptionalVariableName, UPARAM(ref) FTransform& NewValue);

    /**
     * Sets an FVector.
     *
     * @param OptionalVariableName The name of an uproperty inside the uobject that this function is being called on.
     * @param NewValue The new FVector.
     * @return The updated FVector.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Vector")
    FVector SetVectorSingleObjectVariable(const FName OptionalVariableName, UPARAM(ref) FVector& NewValue);
};