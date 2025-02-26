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

#include "ObjectVariables/GorgeousObjectVariable.h"

#include "ObjectVariables/GorgeousRootObjectVariable.h"


UGorgeousObjectVariable::UGorgeousObjectVariable(): VariableRegistry(TArray<TObjectPtr<UGorgeousObjectVariable>>()),
                                                    bPersistent(false),
                                                    Parent(nullptr)
{
}

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
	NewObjectVariable->UniqueVariableIdentifier = NewObjectVariableIdentifier;
	NewObjectVariable->Parent = InParent;
	NewObjectVariable->bPersistent = bShouldPersist;
	InParent->RegisterWithRegistry(NewObjectVariable);

	UGorgeousLoggingBlueprintFunctionLibrary::LogSuccessMessage(FString::Printf(TEXT("Successfully registered new object variable with identifier: %s where the parent is: %s (%s)"),
		*Identifier.ToString(), *InParent->GetName(), *InParent->UniqueVariableIdentifier.ToString()), "GT.ObjectVariables.New");
	
	return NewObjectVariable;
}

template <typename InTCppType, typename TInPropertyBaseClass>
void UGorgeousObjectVariable::SetDynamicProperty(const FName PropertyName, const InTCppType& Value)
{
	if (FProperty* Property = FindFProperty<FProperty>(GetClass(), PropertyName); PropertyName && Property->IsA(TProperty<InTCppType, TInPropertyBaseClass>()))
	{
		if (TProperty<InTCppType, TInPropertyBaseClass>* TypedProperty = CastField<TProperty<InTCppType, TInPropertyBaseClass>>(Property))
		{
			TypedProperty->SetPropertyValue_InContainer(this, Value);
		}
	}
}

//     MyActor->SetDynamicProperty<int32, FIntProperty>(FName("MyIntProperty"), 42);

/**
 * 
	int32 RetrievedValue;
	if (MyActor && MyActor->GetDynamicProperty<int32, FIntProperty>(FName("MyIntProperty"), RetrievedValue))
	{
		UE_LOG(LogTemp, Log, TEXT("Retrieved Value: %d"), RetrievedValue);
	}
 */


template <typename InTCppType, typename TInPropertyBaseClass>
bool UGorgeousObjectVariable::GetDynamicProperty(const FName PropertyName, InTCppType& OutValue) const
{
	FProperty* Property = FindFProperty<FProperty>(GetClass(), PropertyName);
	if (Property && Property->IsA<TProperty<InTCppType, TInPropertyBaseClass>>())
	{
		if (const TProperty<InTCppType, TInPropertyBaseClass>* TypedProperty = CastChecked<TProperty<InTCppType, TInPropertyBaseClass>>(Property))
		{
			OutValue = TypedProperty->GetPropertyValue_InContainer(this);
			return true;
		}
	}
	return false;
}

