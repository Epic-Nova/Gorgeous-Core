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
#include "GorgeousMapObjectVariablesGetter_I.generated.h"

class UGorgeousObjectVariable;

UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousMapObjectVariablesGetter_I : public UInterface
{
	GENERATED_BODY()
};

class GORGEOUSCORERUNTIME_API IGorgeousMapObjectVariablesGetter_I
{
	GENERATED_BODY()

public:
	
	//For Map Object Variables
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object Variable")
	TMap<UGorgeousObjectVariable*, UGorgeousObjectVariable*> GetObjectVariableMapObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object")
	TMap<UObject*, UGorgeousObjectVariable*> GetObjectObjectMapObjectVariable() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object")
	TMap<UClass*, UGorgeousObjectVariable*> GetObjectClassMapObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object")
	TMap<TSoftObjectPtr<UObject>, UGorgeousObjectVariable*> GetSoftObjectObjectMapObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Object")
	TMap<TSoftClassPtr<UObject>, UGorgeousObjectVariable*> GetSoftObjectClassMapObjectVariable() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Byte")
	TMap<uint8, UGorgeousObjectVariable*> GetByteMapObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Float")
	TMap<double, UGorgeousObjectVariable*> GetFloatMapObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Integer64")
	TMap<int64, UGorgeousObjectVariable*> GetInteger64MapObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Integer")
	TMap<int32, UGorgeousObjectVariable*> GetIntegerMapObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Name")
	TMap<FName, UGorgeousObjectVariable*> GetNameMapObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|String")
	TMap<FString, UGorgeousObjectVariable*> GetStringMapObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Transform")
	TMap<FTransform, UGorgeousObjectVariable*> GetTransformMapObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Map|Vector")
	TMap<FVector, UGorgeousObjectVariable*> GetVectorMapObjectVariable() const;
};