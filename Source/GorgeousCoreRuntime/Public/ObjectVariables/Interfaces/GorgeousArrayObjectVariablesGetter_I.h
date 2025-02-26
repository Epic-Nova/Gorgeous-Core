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
#include "GorgeousArrayObjectVariablesGetter_I.generated.h"

class UGorgeousObjectVariable;

UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousArrayObjectVariablesGetter_I : public UInterface
{
	GENERATED_BODY()
};

class GORGEOUSCORERUNTIME_API IGorgeousArrayObjectVariablesGetter_I
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object Variable")
	TArray<UGorgeousObjectVariable*> GetObjectVariableArrayObjectVariable() const;

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object")
	TArray<UObject*> GetObjectObjectArrayObjectVariable() const;
	
	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object")
	TArray<UClass*> GetObjectClassArrayObjectVariable() const;

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object")
	TArray<TSoftObjectPtr<UObject>> GetSoftObjectObjectArrayObjectVariable() const;
	
	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Object")
	TArray<TSoftClassPtr<UObject>> GetSoftObjectClassArrayObjectVariable() const;

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Boolean")
	TArray<bool> GetBooleanArrayObjectVariable() const;

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Byte")
	TArray<uint8> GetByteArrayObjectVariable() const;

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Float")
	TArray<double> GetFloatArrayObjectVariable() const;

		/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Integer")
	TArray<int32> GetIntegerArrayObjectVariable() const;

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Integer64")
	TArray<int64> GetInteger64ArrayObjectVariable() const;

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Name")
	TArray<FName> GetNameArrayObjectVariable() const;

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Rotator")
	TArray<FRotator> GetRotatorArrayObjectVariable() const;

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|String")
	TArray<FString> GetStringArrayObjectVariable() const;

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Text")
	TArray<FText> GetTextArrayObjectVariable() const;

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Transform")
	TArray<FTransform> GetTransformArrayObjectVariable() const;

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Array|Vector")
	TArray<FVector> GetVectorArrayObjectVariable() const;
};