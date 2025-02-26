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
#pragma once

#include "UObject/Interface.h"
#include "GorgeousArrayObjectVariablesSetter_I.generated.h"

class UGorgeousObjectVariable;

UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousArrayObjectVariablesSetter_I : public UInterface
{
	GENERATED_BODY()
};

class GORGEOUSCORERUNTIME_API IGorgeousArrayObjectVariablesSetter_I
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object Variable")
	TArray<UGorgeousObjectVariable*> SetObjectVariableArrayObjectVariable(UPARAM(ref) TArray<UGorgeousObjectVariable*>& NewValue);

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object")
	TArray<UObject*> SetObjectObjectArrayObjectVariable(UPARAM(ref) TArray<UObject*>& NewValue);

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object")
	TArray<UClass*> SetObjectClassArrayObjectVariable(UPARAM(ref) TArray<UClass*>& NewValue);

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object")
	TArray<TSoftObjectPtr<UObject>> SetSoftObjectObjectArrayObjectVariable(UPARAM(ref) TArray<TSoftObjectPtr<UObject>>& NewValue);

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object")
	TArray<TSoftClassPtr<UObject>> SetSoftObjectClassArrayObjectVariable(UPARAM(ref) TArray<TSoftClassPtr<UObject> >& NewValue);

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Boolean")
	TArray<bool> SetBooleanArrayObjectVariable(UPARAM(ref) TArray<bool>& NewValue);
	
	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Byte")
	TArray<uint8> SetByteArrayObjectVariable(UPARAM(ref) TArray<uint8>& NewValue);

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Float")
	TArray<double> SetFloatArrayObjectVariable(UPARAM(ref) TArray<double>& NewValue);
	
	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Integer")
	TArray<int32> SetIntegerArrayObjectVariable(UPARAM(ref) TArray<int32>& NewValue);

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Integer64")
	TArray<int64> SetInteger64ArrayObjectVariable(UPARAM(ref) TArray<int64>& NewValue);

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Name")
	TArray<FName> SetNameArrayObjectVariable(UPARAM(ref) TArray<FName>& NewValue);

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Rotator")
	TArray<FRotator> SetRotatorArrayObjectVariable(UPARAM(ref) TArray<FRotator>& NewValue);

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|String")
	TArray<FString> SetStringArrayObjectVariable(UPARAM(ref) TArray<FString>& NewValue);

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Text")
	TArray<FText> SetTextArrayObjectVariable(UPARAM(ref) TArray<FText>& NewValue);

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Transform")
	TArray<FTransform> SetTransformArrayObjectVariable(UPARAM(ref) TArray<FTransform>& NewValue);

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Vector")
	TArray<FVector> SetVectorArrayObjectVariable(UPARAM(ref) TArray<FVector>& NewValue);
};