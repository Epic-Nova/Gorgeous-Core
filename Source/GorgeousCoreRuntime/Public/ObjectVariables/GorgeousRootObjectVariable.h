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

#include "GorgeousObjectVariable.h"
#include "GorgeousRootObjectVariable.generated.h"


//@TODO: A console command that prints out the whole hierarchy of the variables
UCLASS(BlueprintType)
class UGorgeousRootObjectVariable : public UGorgeousObjectVariable
{
	GENERATED_BODY()

public:

	UGorgeousRootObjectVariable();
	
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables")
	static UGorgeousRootObjectVariable* GetRootObjectVariable();

	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables")
	static TArray<UGorgeousObjectVariable*> GetVariableHierarchyRegistry();

	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Gorgeous Object Variables")
	static TArray<UGorgeousObjectVariable*> GetRootVariableRegistry();

	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Gorgeous Object Variables")
	static void RemoveVariableFromRegistry(UGorgeousObjectVariable* VariableToRemove);

	static void CleanupRegistry(bool bFullCleanup = false);
	
	/**
	 * Used to set the value of a property with any type you want. The requirement for this is that a property with the same name and type exists in the requesting class.
	 * To be able to get access to the object variable it has to be spawned through the NewObjectVariable function or any function that assigns a unique FGuid to its tags upon spawn.
	 * 
	 * @param Identifier The unique identifier that got assigned to the object variable in the spawn process commonly returned by the function NewObjectVaraible
	 * @param Value Any value that you want to set. Be creative, anything works!
	 * @param OptionalPropertyName Optionally the name of a property that is contained inside the object variable instance. Defaults to "Value" for the default value of an object variable.
	 *
	 * @todo A dropdown that lists the available variables inside OptionalPropertyName with a corresponding TSubclassOf (meta specifier)
	 * @todo dynamic pin for the variable type (right click, change to variable type => like the math operations) or automatically set the thunk type with the OptionalPropertyName or default value "Value"
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Gorgeous Core|Gorgeous Object Variables", meta = (CustomStructureParam = "Value"))
	static void SetUniversalVariable(FGuid Identifier, FName OptionalPropertyName, const int32& Value);

	DECLARE_FUNCTION(execSetUniversalVariable)
	{
		P_GET_STRUCT(FGuid, Identifier);
		P_GET_PROPERTY(FNameProperty, OptionalPropertyName);
		Stack.StepCompiledIn<FProperty>(nullptr);
		const FProperty* SourceProperty  = Stack.MostRecentProperty;
		const void* SourcePropertyAddress  = Stack.MostRecentPropertyAddress;
		P_FINISH;
		
		if (OptionalPropertyName.IsNone())
		{
			OptionalPropertyName = "Value";
		}

		UGorgeousObjectVariable* FoundObjectVariable = nullptr;
			
		for (const auto ObjectVariable : GetVariableHierarchyRegistry())
		{
			if (ObjectVariable->UniqueVariableIdentifier == Identifier)
			{
				FoundObjectVariable = ObjectVariable;
				break;
			}
		}

		if (FoundObjectVariable && SourceProperty && SourcePropertyAddress)
		{
			if (const FProperty* TargetProperty = FindFProperty<FProperty>(FoundObjectVariable->GetClass(), OptionalPropertyName))
			{
				if (TargetProperty->SameType(SourceProperty))
				{
					TargetProperty->SetValue_InContainer(FoundObjectVariable, SourcePropertyAddress);
				}
				else
				{
					UGorgeousLoggingBlueprintFunctionLibrary::LogWarningMessage(FString::Printf(TEXT("Property type mismatch for %s"), *OptionalPropertyName.ToString()), "GT.ObjectVariables.Universal.Type_Mismatch", 2.f, Stack.Object);
				}
			}
		}
	}
	
	/**
	 * Used to receive the value of a property with any type you want. The requirement for this is that a property with the same name and type exists in the requesting class.
	 * To be able to get access to the object variable it has to be spawned through the NewObjectVariable function or any function that assigns a unique FGuid to its tags upon spawn.
	 * 
	 * @param Identifier The unique identifier that got assigned to the object variable in the spawn process commonly returned by the function NewObjectVariable
	 * @param OutValue Any value that you want to set. Be creative, anything works!
	 * @param OptionalPropertyName Optionally the name of a property that is contained inside the object variable instance. Defaults to "Value" for the default value of an object variable.
	 *
	 * @todo A dropdown that lists the available variables inside OptionalPropertyName with a corresponding TSubclassOf (meta specifier)
	 * @todo dynamic pin for the variable type (right click, change to variable type => like the math operations) or automatically set the thunk type with the OptionalPropertyName or default value "Value"
	 */
	UFUNCTION(BlueprintPure, CustomThunk, Category = "Gorgeous Core|Gorgeous Object Variables", meta = (CustomStructureParam = "OutValue"))
	static void GetUniversalVariable(FGuid Identifier, FName OptionalPropertyName, int32& OutValue);

	DECLARE_FUNCTION(execGetUniversalVariable)
	{
		P_GET_STRUCT(FGuid, Identifier);
		P_GET_PROPERTY(FNameProperty, OptionalPropertyName);
		Stack.StepCompiledIn<FProperty>(nullptr);
		void* OutValueAddress = Stack.MostRecentPropertyAddress;
		const FProperty* OutValueProperty = Stack.MostRecentProperty;
		P_FINISH;

		if (OptionalPropertyName.IsNone())
		{
			OptionalPropertyName = "Value";
		}
		
		UGorgeousObjectVariable* FoundObjectVariable = nullptr;
			
		for (const auto ObjectVariable : GetVariableHierarchyRegistry())
		{
			if (ObjectVariable->UniqueVariableIdentifier == Identifier)
			{
				FoundObjectVariable = ObjectVariable;
				break;
			}
		}
		
		if (FoundObjectVariable)
		{
			if (const FProperty* SourceProperty = FindFProperty<FProperty>(FoundObjectVariable->GetClass(), OptionalPropertyName))
			{
				if (SourceProperty->SameType(OutValueProperty))
				{
					SourceProperty->CopyCompleteValue(OutValueAddress, SourceProperty->ContainerPtrToValuePtr<void>(FoundObjectVariable));
				}
				else
				{
					UGorgeousLoggingBlueprintFunctionLibrary::LogWarningMessage(FString::Printf(TEXT("Property type mismatch for %s"), *OptionalPropertyName.ToString()), "GT.ObjectVariables.Universal.Type_Mismatch", 2.f, Stack.Object);
				}
			}
		}
	}
	
	virtual void RegisterWithRegistry(TObjectPtr<UGorgeousObjectVariable> NewObjectVariable) override;

protected:

	static TArray<TObjectPtr<UGorgeousObjectVariable>> RootVariableRegistry;

	static TObjectPtr<UGorgeousRootObjectVariable> SingletonRootInstance;
	
};
