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

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Kismet/GameplayStatics.h"

#include "GorgeousCoreUtilitiesMinimalShared.h"

#include "ObjectVariables/Helpers/Macros/GorgeousObjectVariableHelperMacros.h"
#include "ObjectVariables/Interfaces/GorgeousSingleObjectVariablesGetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousSingleObjectVariablesSetter_I.h"

#include "ObjectVariables/Interfaces/GorgeousArrayObjectVariablesGetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousArrayObjectVariablesSetter_I.h"

#include "ObjectVariables/Interfaces/GorgeousMapObjectVariablesGetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousMapObjectVariablesSetter_I.h"

#include "ObjectVariables/Interfaces/GorgeousSetObjectVariablesGetter_I.h"
#include "ObjectVariables/Interfaces/GorgeousSetObjectVariablesSetter_I.h"

#include "GorgeousObjectVariable.generated.h"

/*==================================================================================================>
| Class that provides the functionality to define variables in object form.							|
| Children classes will be able to replicate its value over the network.							|
<==================================================================================================*/
UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, ClassGroup = "Gorgeous Core|Gorgeous Object Variables", DisplayName = "Gorgeous Object Variable", Category = "Gorgeous Core|Gorgeous Object Variables",
	meta = (ToolTip = "Used for providing a more interactive way to define variables in object form.", ShortTooltip = "The base class for all object variables."))
class GORGEOUSCORERUNTIME_API UGorgeousObjectVariable : public UObject,
public IGorgeousSingleObjectVariablesGetter_I, public IGorgeousSingleObjectVariablesSetter_I,
public IGorgeousArrayObjectVariablesGetter_I, public IGorgeousArrayObjectVariablesSetter_I,
public IGorgeousMapObjectVariablesGetter_I, public IGorgeousMapObjectVariablesSetter_I,
public IGorgeousSetObjectVariablesGetter_I, public IGorgeousSetObjectVariablesSetter_I
{
	GENERATED_BODY()
	
	/*==============================>
	| Functions Overrides & Events	|
	<==============================*/
protected:
	
	/** The Class Constructor for the Base Object Variable is used to set Default Values. */
	UGorgeousObjectVariable();

	virtual bool IsSupportedForNetworking() const override { return true; }

public:

	virtual void RegisterWithRegistry(TObjectPtr<UGorgeousObjectVariable> NewObjectVariable);
	
   template<typename InTCppType, typename TInPropertyBaseClass>
	void SetDynamicProperty(const FName PropertyName, const InTCppType& Value);

    template<typename InTCppType, typename TInPropertyBaseClass>
    bool GetDynamicProperty(const FName PropertyName, InTCppType& OutValue) const;

	/**
	 * Constructs a new object variable and registers it within the given registry depending on the parent given.
	 * 
	 * @param InClass the class that the object variable should derive from
	 * @param Identifier the unique identifier of the object variable
	 * @param Parent THe parent of this object variable. The chain can be followed up to the root object variable
	 * @param bShouldPersist Weather this object variable should be persistent across level switches
	 * @return a new variable in object format
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "Gorgeous Core|Gorgeous Object Variables")
	UGorgeousObjectVariable* NewObjectVariable(TSubclassOf<UGorgeousObjectVariable> Class, FGuid& Identifier, UGorgeousObjectVariable* Parent, bool bShouldPersist);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGuid UniqueVariableIdentifier;

	TArray<TObjectPtr<UGorgeousObjectVariable>> VariableRegistry;

	bool bPersistent;
	
protected:
	UPROPERTY(VisibleAnywhere, SaveGame, Category = "Default")
	UGorgeousObjectVariable* Parent;

	
private:
	
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(UGorgeousObjectVariable*, ObjectVariable, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(UObject*, ObjectObject, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(UClass*, ObjectClass, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSoftObjectPtr<UObject>, SoftObjectObject, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSoftClassPtr<UObject>, SoftObjectClass, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(bool, Boolean, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(uint8, Byte, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(double, Float, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(int64, Integer64, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(int32, Integer, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(FName, Name, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(FRotator, Rotator, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(FString, String, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(FText, Text, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(FTransform, Transform, Single)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(FVector, Vector, Single)

	
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<UGorgeousObjectVariable*>, ObjectVariable, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<UObject*>, ObjectObject, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<UClass*>, ObjectClass, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<TSoftObjectPtr<UObject>>, SoftObjectObject, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<TSoftClassPtr<UObject>>, SoftObjectClass, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<bool>, Boolean, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<uint8>, Byte, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<double>, Float, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<int64>, Integer64, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<int32>, Integer, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<FName>, Name, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<FRotator>, Rotator, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<FString>, String, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<FText>, Text, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<FTransform>, Transform, Array)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TArray<FVector>, Vector, Array)

	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(UGorgeousObjectVariable*, ObjectVariable)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(UObject*, ObjectObject)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(UClass*, ObjectClass)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(TSoftObjectPtr<UObject>, SoftObjectObject)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(TSoftClassPtr<UObject>, SoftObjectClass)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(uint8, Byte)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(double, Float)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(int64, Integer64)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(int32, Integer)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(FName, Name)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(FString, String)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(FTransform, Transform)
	UE_DEFINE_OBJECT_VARIABLE_MAP_REFERENCE_INTERFACE(FVector, Vector)

	
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<UGorgeousObjectVariable*>, ObjectVariable, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<UObject*>, ObjectObject, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<UClass*>, ObjectClass, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<TSoftObjectPtr<UObject>>, SoftObjectObject, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<TSoftClassPtr<UObject>>, SoftObjectClass, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<uint8>, Byte, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<double>, Float, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<int64>, Integer64, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<int32>, Integer, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<FName>, Name, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<FString>, String, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<FTransform>, Transform, Set)
	UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_REFERENCE_INTERFACE(TSet<FVector>, Vector, Set)
};


