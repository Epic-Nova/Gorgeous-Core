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
#include "GorgeousCoreUtilitiesMinimalShared.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousObjectVariable Implementation
//=============================================================================

UGorgeousObjectVariable::UGorgeousObjectVariable(): VariableRegistry(TArray<TObjectPtr<UGorgeousObjectVariable>>())
													, bPersistent(false)
													,Parent(nullptr) {}

UGorgeousObjectVariable* UGorgeousObjectVariable::NewObjectVariable(const TSubclassOf<UGorgeousObjectVariable> Class, FGuid& Identifier, UGorgeousObjectVariable* InParent, const bool bShouldPersist)
{
	if (!Class && Class->IsValidLowLevel())
	{
		GT_E_LOG("GT.ObjectVariables.Registration.Successful", TEXT("You are trying to register a object variable without a valid class, check if the class is valid!"));
		return nullptr;
	}

	if (!InParent)
	{
		InParent = UGorgeousRootObjectVariable::GetRootObjectVariable();
		GT_I_LOG("GT.ObjectVariables.No_Parent", TEXT("No parent were specified, therefore the root object variable will be used as the parent"));
	}
	
	UGorgeousObjectVariable* NewObjectVariable = NewObject<UGorgeousObjectVariable>(InParent, Class);
	NewObjectVariable->AddToRoot();
	
	const FGuid NewObjectVariableIdentifier = FGuid::NewGuid();
	Identifier = NewObjectVariableIdentifier;
	NewObjectVariable->UniqueIdentifier = NewObjectVariableIdentifier;
	
	NewObjectVariable->Parent = InParent;
	NewObjectVariable->bPersistent = bShouldPersist;
	InParent->RegisterWithRegistry(NewObjectVariable);

	GT_S_LOG("GT.ObjectVariables.Registration.Successful", TEXT("Successfully registered object variable with identifier: %s where the parent is: %s (%s)"),
		 *Identifier.ToString(), 
		 *InParent->GetName(), 
		 *InParent->UniqueIdentifier.ToString());
	
	return NewObjectVariable;
}

UGorgeousObjectVariable* UGorgeousObjectVariable::InstantiateTransactionalObjectVariable(
	const TSubclassOf<UGorgeousObjectVariable> Class, UGorgeousObjectVariable* InParent, UObject* Outer)
{
	if (!Class)
	{
		GT_E_LOG("GT.ObjectVariables.Transactional.Failed", TEXT("Failed to create new transactional instance"));
		return nullptr;
	}

	if (!InParent)
	{
		InParent = UGorgeousRootObjectVariable::GetRootObjectVariable();
		GT_I_LOG("GT.ObjectVariables.No_Parent", TEXT("No parent were specified, therefore the root object variable will be used as the parent"));
	}
	
	UGorgeousObjectVariable* NewInstance = NewObject<UGorgeousObjectVariable>(Outer ? Outer : InParent, Class, NAME_None, RF_Transactional);
	NewInstance->UniqueIdentifier = FGuid::NewGuid();
	NewInstance->Parent = InParent;
	
	if (NewInstance)
	{
		Modify();

		GT_S_LOG("GT.ObjectVariables.Transactional.Success", TEXT("Created new transactional instance."));
		return NewInstance;
	}
	GT_E_LOG("GT.ObjectVariables.Transactional.Failed", TEXT("Failed to create new transactional instance"));
	return nullptr;
}

void UGorgeousObjectVariable::InvokeInstancedFunctionality(const FGuid NewUniqueIdentifier)
{
	if (!UGorgeousRootObjectVariable::IsVariableRegistered(this) && NewUniqueIdentifier.IsValid())
	{
		if (!Parent)
		{
			Parent = UGorgeousRootObjectVariable::GetRootObjectVariable();
			GT_I_LOG("GT.ObjectVariables.No_Parent", TEXT("No parent were specified, therefore the root object variable will be used as the parent"));
		}
		UniqueIdentifier = NewUniqueIdentifier;
		Parent->RegisterWithRegistry(this);
	}
}

void UGorgeousObjectVariable::SetParent(UGorgeousObjectVariable* NewParent)
{
	Parent = NewParent;
	Rename(*GetName(), NewParent);
}

void UGorgeousObjectVariable::RegisterWithRegistry(UGorgeousObjectVariable* NewObjectVariable)
{
	if (NewObjectVariable && !UGorgeousRootObjectVariable::IsVariableRegistered(NewObjectVariable))
	{
		VariableRegistry.Add(NewObjectVariable);
	}
}

FGuid UGorgeousObjectVariable::GetUniqueIdentifierForObjectVariable_Implementation()
{
	return UniqueIdentifier;
}
