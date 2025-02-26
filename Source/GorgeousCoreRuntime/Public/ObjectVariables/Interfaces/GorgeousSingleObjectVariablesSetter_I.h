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
#include "GorgeousSingleObjectVariablesSetter_I.generated.h"

class UGorgeousObjectVariable;

UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousSingleObjectVariablesSetter_I : public UInterface
{
	GENERATED_BODY()
};

class GORGEOUSCORERUNTIME_API IGorgeousSingleObjectVariablesSetter_I
{
	GENERATED_BODY()

public:

	//For Single Object Variables

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object Variable")
	UGorgeousObjectVariable* SetObjectVariableSingleObjectVariable(UPARAM(ref) UGorgeousObjectVariable*& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
	UObject* SetObjectObjectSingleObjectVariable(UPARAM(ref) UObject*& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
	UClass* SetObjectClassSingleObjectVariable(UPARAM(ref) UClass*& NewValue);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
	TSoftObjectPtr<UObject> SetSoftObjectObjectSingleObjectVariable(UPARAM(ref) TSoftObjectPtr<UObject>& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
	TSoftClassPtr<UObject> SetSoftObjectClassSingleObjectVariable(UPARAM(ref) TSoftClassPtr<UObject>& NewValue);

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Boolean")
	bool SetBooleanSingleObjectVariable(UPARAM(ref) bool& NewValue);

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Byte")
	uint8 SetByteSingleObjectVariable(UPARAM(ref) uint8& NewValue);

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Float")
	double SetFloatSingleObjectVariable(UPARAM(ref) double& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Integer64")
	int64 SetInteger64SingleObjectVariable(UPARAM(ref) int64& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Integer")
	int32 SetIntegerSingleObjectVariable(UPARAM(ref) int32& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Name")
	FName SetNameSingleObjectVariable(UPARAM(ref) FName& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Rotator")
	FRotator SetRotatorSingleObjectVariable(UPARAM(ref) FRotator& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|String")
	FString SetStringSingleObjectVariable(UPARAM(ref) FString& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Text")
	FText SetTextSingleObjectVariable(UPARAM(ref) FText& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Transform")
	FTransform SetTransformSingleObjectVariable(UPARAM(ref) FTransform& NewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Vector")
	FVector SetVectorSingleObjectVariable(UPARAM(ref) FVector& NewValue);
};