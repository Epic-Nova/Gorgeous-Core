/*==========================================================================>
|				Gorgeous Core - Core functionality provider					|
| ------------------------------------------------------------------------- |
|		  Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,			|
|			  administrated by Epic Nova. All rights reserved.				|
| ------------------------------------------------------------------------- |
|					Epic Nova is an independent entity,						|
|	    that is has nothing in common with Epic Games in any capacity.		|
<==========================================================================*/
#pragma once

#include "UObject/Interface.h"
#include "GorgeousSetObjectVariablesSetter_I.generated.h"

class UGorgeousObjectVariable;

UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousSetObjectVariablesSetter_I : public UInterface
{
	GENERATED_BODY()
};

class GORGEOUSCORERUNTIME_API IGorgeousSetObjectVariablesSetter_I
{
	GENERATED_BODY()

public:

	//For Set Object Variables

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object Variable")
	TSet<UGorgeousObjectVariable*> SetObjectVariableSetObjectVariable(UPARAM(ref) TSet<UGorgeousObjectVariable*>& NewValue);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object")
	TSet<UObject*> SetObjectObjectSetObjectVariable(UPARAM(ref) TSet<UObject*>& NewValue);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object")
	TSet<UClass*> SetObjectClassSetObjectVariable(UPARAM(ref) TSet<UClass*>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object")
	TSet<TSoftObjectPtr<UObject>> SetSoftObjectObjectSetObjectVariable(UPARAM(ref) TSet<TSoftObjectPtr<UObject>>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Object")
	TSet<TSoftClassPtr<UObject>> SetSoftObjectClassSetObjectVariable(UPARAM(ref) TSet<TSoftClassPtr<UObject>>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Byte")
	TSet<uint8> SetByteSetObjectVariable(UPARAM(ref) TSet<uint8>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Float")
	TSet<double> SetFloatSetObjectVariable(UPARAM(ref) TSet<double>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Integer64")
	TSet<int64> SetInteger64SetObjectVariable(UPARAM(ref) TSet<int64>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Integer")
	TSet<int32> SetIntegerSetObjectVariable(UPARAM(ref) TSet<int32>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Name")
	TSet<FName> SetNameSetObjectVariable(UPARAM(ref) TSet<FName>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|String")
	TSet<FString> SetStringSetObjectVariable(UPARAM(ref) TSet<FString>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Transform")
	TSet<FTransform> SetTransformSetObjectVariable(UPARAM(ref) TSet<FTransform>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Set|Vector")
	TSet<FVector> SetVectorSetObjectVariable(UPARAM(ref) TSet<FVector>& NewValue);
};