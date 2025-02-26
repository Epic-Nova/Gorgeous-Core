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
#include "GorgeousSingleObjectVariablesGetter_I.generated.h"

class UGorgeousObjectVariable;

UINTERFACE(MinimalAPI, BlueprintType)
class UGorgeousSingleObjectVariablesGetter_I : public UInterface
{
	GENERATED_BODY()
};

class GORGEOUSCORERUNTIME_API IGorgeousSingleObjectVariablesGetter_I
{
	GENERATED_BODY()

public:
		//For Single Object Variables

	/** Please add a function description */
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object Variable")
	UGorgeousObjectVariable* GetObjectVariableSingleObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
	UObject* GetObjectObjectSingleObjectVariable() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
	UClass* GetObjectClassSingleObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
	TSoftObjectPtr<UObject> GetSoftObjectObjectSingleObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Object")
	TSoftClassPtr<UObject> GetSoftObjectClassSingleObjectVariable() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables|Single|Boolean")
	bool GetBooleanSingleObjectVariable() const;

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Byte")
	uint8 GetByteSingleObjectVariable() const;

	/** Please add a function description */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Float")
	double GetFloatSingleObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Integer64")
	int64 GetInteger64SingleObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Integer")
	int32 GetIntegerSingleObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Name")
	FName GetNameSingleObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Rotator")
	FRotator GetRotatorSingleObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|String")
	FString GetStringSingleObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Text")
	FText GetTextSingleObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Transform")
	FTransform GetTransformSingleObjectVariable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Gorgeous Core|Gorgeous Object Variables|Single|Vector")
	FVector GetVectorSingleObjectVariable() const;
};