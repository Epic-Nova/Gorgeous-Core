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
#include "GorgeousMapObjectVariablesSetter_I.generated.h"

class UGorgeousObjectVariable;

UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousMapObjectVariablesSetter_I : public UInterface
{
	GENERATED_BODY()
};

class GORGEOUSCORERUNTIME_API IGorgeousMapObjectVariablesSetter_I
{
	GENERATED_BODY()

public:
	//For Map Object Variables

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object Variable")
	TMap<UGorgeousObjectVariable*, UGorgeousObjectVariable*> SetObjectVariableMapObjectVariable(UPARAM(ref) TMap<UGorgeousObjectVariable*, UGorgeousObjectVariable*>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object")
	TMap<UObject*, UGorgeousObjectVariable*> SetObjectObjectMapObjectVariable(UPARAM(ref) TMap<UObject*, UGorgeousObjectVariable*>& NewValue);
	
	TMap<UClass*, UGorgeousObjectVariable*> GetObjectClassMapObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object")
	TMap<UClass*, UGorgeousObjectVariable*> SetObjectClassMapObjectVariable(UPARAM(ref) TMap<UClass*, UGorgeousObjectVariable*>& NewValue);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object")
	TMap<TSoftObjectPtr<UObject>, UGorgeousObjectVariable*> SetSoftObjectObjectMapObjectVariable(UPARAM(ref) TMap<TSoftObjectPtr<UObject>, UGorgeousObjectVariable*>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object")
	TMap<TSoftClassPtr<UObject>, UGorgeousObjectVariable*> SetSoftObjectClassMapObjectVariable(UPARAM(ref) TMap<TSoftClassPtr<UObject>, UGorgeousObjectVariable*>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Byte")
	TMap<uint8, UGorgeousObjectVariable*> SetByteMapObjectVariable(UPARAM(ref) TMap<uint8, UGorgeousObjectVariable*>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Float")
	TMap<double, UGorgeousObjectVariable*> SetFloatMapObjectVariable(UPARAM(ref) TMap<double, UGorgeousObjectVariable*>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Integer64")
	TMap<int64, UGorgeousObjectVariable*> SetInteger64MapObjectVariable(UPARAM(ref) TMap<int64, UGorgeousObjectVariable*>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Integer")
	TMap<int32, UGorgeousObjectVariable*> SetIntegerMapObjectVariable(UPARAM(ref) TMap<int32, UGorgeousObjectVariable*>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Name")
	TMap<FName, UGorgeousObjectVariable*> SetNameMapObjectVariable(UPARAM(ref) TMap<FName, UGorgeousObjectVariable*>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|String")
	TMap<FString, UGorgeousObjectVariable*> SetStringMapObjectVariable(UPARAM(ref) TMap<FString, UGorgeousObjectVariable*>& NewValue);

	UFUNCTION(NotBlueprintable, BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Transform")
	TMap<FTransform, UGorgeousObjectVariable*> SetTransformMapObjectVariable(UPARAM(ref) TMap<FTransform, UGorgeousObjectVariable*>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Vector")
	TMap<FVector, UGorgeousObjectVariable*> SetVectorMapObjectVariable(UPARAM(ref) TMap<FVector, UGorgeousObjectVariable*>& NewValue);
};