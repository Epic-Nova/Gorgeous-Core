// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

#include "ObjectVariables/GorgeousObjectVariable.h"

//<=============================--- Includes ---=============================>
//<-------------------------=== Module Includes ===-------------------------->
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "GorgeousLoggingBlueprintFunctionLibrary.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousObjectVariable Implementation
//=============================================================================

UGorgeousObjectVariable::UGorgeousObjectVariable(): VariableRegistry(TArray<TObjectPtr<UGorgeousObjectVariable>>()),
                                                    bPersistent(false),
                                                    Parent(nullptr) {}

void UGorgeousObjectVariable::RegisterWithRegistry(TObjectPtr<UGorgeousObjectVariable> NewObjectVariable)
{
	VariableRegistry.Add(NewObjectVariable);
}

UGorgeousObjectVariable* UGorgeousObjectVariable::NewObjectVariable(const TSubclassOf<UGorgeousObjectVariable> Class, FGuid& Identifier, UGorgeousObjectVariable* InParent, bool bShouldPersist)
{
	if (!Class && Class->IsValidLowLevel())
	{
		UGorgeousLoggingBlueprintFunctionLibrary::LogErrorMessage("You are trying to register a object variable without a valid class, check if the class is valid!", "GT.ObjectVariables.New.Invalid_Class");
		return nullptr;
	}
	
	if (!InParent)
	{
		InParent = UGorgeousRootObjectVariable::GetRootObjectVariable();
		UGorgeousLoggingBlueprintFunctionLibrary::LogInformationMessage("No parent were specified, therefore the root object variable will be used as the parent", "GT.ObjectVariables.New.No_Parent");
	}
	
	UGorgeousObjectVariable* NewObjectVariable = NewObject<UGorgeousObjectVariable>(InParent, Class);
	NewObjectVariable->AddToRoot();
	
	const FGuid NewObjectVariableIdentifier = FGuid::NewGuid();
	Identifier = NewObjectVariableIdentifier;
	NewObjectVariable->UniqueIdentifier = NewObjectVariableIdentifier;
	NewObjectVariable->Parent = InParent;
	NewObjectVariable->bPersistent = bShouldPersist;
	InParent->RegisterWithRegistry(NewObjectVariable);

	UGorgeousLoggingBlueprintFunctionLibrary::LogSuccessMessage(FString::Printf(TEXT("Successfully registered new object variable with identifier: %s where the parent is: %s (%s)"),
		*Identifier.ToString(), *InParent->GetName(), *InParent->UniqueIdentifier.ToString()), "GT.ObjectVariables.New");
	
	return NewObjectVariable;
}

void UGorgeousObjectVariable::InvokeInstancedFunctionality()
{
	UGorgeousRootObjectVariable::GetRootObjectVariable()->RegisterWithRegistry(this);
}

template <typename InTCppType, typename TInPropertyBaseClass>
void UGorgeousObjectVariable::SetDynamicProperty(const FName PropertyName, const InTCppType& Value)
{
	if (FProperty* Property = FindFProperty<FProperty>(GetClass(), PropertyName); PropertyName.IsValid())
	{
		// Handle UObject* properties separately
		if constexpr (std::is_base_of_v<UObject, InTCppType>)
		{
			if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
			{
				ObjectProperty->SetObjectPropertyValue_InContainer(this, Value);
			}
		}
		else if (Property && Property->IsA<TProperty<InTCppType, TInPropertyBaseClass>>())
		{
			if (TProperty<InTCppType, TInPropertyBaseClass>* TypedProperty = CastField<TProperty<InTCppType, TInPropertyBaseClass>>(Property))
			{
				TypedProperty->SetPropertyValue_InContainer(this, Value);
			}
		}
	}
}

template <typename InTCppType, typename TInPropertyBaseClass>
bool UGorgeousObjectVariable::GetDynamicProperty(const FName PropertyName, InTCppType& OutValue) const
{
	if (FProperty* Property = FindFProperty<FProperty>(GetClass(), PropertyName); Property && PropertyName.IsValid())
	{
		// Handle UObject* properties separately
		if constexpr (std::is_base_of_v<UObject, InTCppType>)
		{
			if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
			{
				OutValue = Cast<InTCppType>(ObjectProperty->GetObjectPropertyValue_InContainer(this));
				return true;
			}
		}
		else if (Property && Property->IsA<TProperty<InTCppType, TInPropertyBaseClass>>())
		{
			if (const TProperty<InTCppType, TInPropertyBaseClass>* TypedProperty = CastField<TProperty<InTCppType, TInPropertyBaseClass>>(Property))
			{
				OutValue = TypedProperty->GetPropertyValue_InContainer(this);
				return true;
			}
		}
	}
	return false;
}

