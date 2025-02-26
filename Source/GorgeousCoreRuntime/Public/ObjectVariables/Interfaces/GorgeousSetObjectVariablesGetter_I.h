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
#include "GorgeousSetObjectVariablesGetter_I.generated.h"

class UGorgeousObjectVariable;

UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousSetObjectVariablesGetter_I : public UInterface
{
	GENERATED_BODY()
};

class GORGEOUSCORERUNTIME_API IGorgeousSetObjectVariablesGetter_I
{
	GENERATED_BODY()

public:

	//For Set Object Variables
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object Variable")
	TSet<UGorgeousObjectVariable*> GetObjectVariableSetObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object")
	TSet<UObject*> GetObjectObjectSetObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object")
	TSet<UClass*> GetObjectClassSetObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object")
	TSet<TSoftObjectPtr<UObject>> GetSoftObjectObjectSetObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object")
	TSet<TSoftClassPtr<UObject>> GetSoftObjectClassSetObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Byte")
	TSet<uint8> GetByteSetObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Float")
	TSet<double> GetFloatSetObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Integer64")
	TSet<int64> GetInteger64SetObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Integer")
	TSet<int32> GetIntegerSetObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Name")
	TSet<FName> GetNameSetObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|String")
	TSet<FString> GetStringSetObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Transform")
	TSet<FTransform> GetTransformSetObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Vector")
	TSet<FVector> GetVectorSetObjectVariable() const;
};