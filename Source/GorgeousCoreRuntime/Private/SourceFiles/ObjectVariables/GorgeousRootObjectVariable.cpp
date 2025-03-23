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

#include "ObjectVariables/GorgeousRootObjectVariable.h"

//=============================================================================
// UGorgeousRootObjectVariable Implementation
//=============================================================================

TArray<TObjectPtr<UGorgeousObjectVariable>> UGorgeousRootObjectVariable::RootVariableRegistry = TArray<TObjectPtr<UGorgeousObjectVariable>>();
TObjectPtr<UGorgeousRootObjectVariable> UGorgeousRootObjectVariable::SingletonRootInstance = nullptr;

UGorgeousRootObjectVariable::UGorgeousRootObjectVariable()
{
	bPersistent = true;
}

UGorgeousRootObjectVariable* UGorgeousRootObjectVariable::GetRootObjectVariable()
{
	if (!SingletonRootInstance)
	{
		SingletonRootInstance = NewObject<UGorgeousRootObjectVariable>();
		SingletonRootInstance->AddToRoot();
		SingletonRootInstance->UniqueIdentifier = FGuid::NewGuid();
	}
	return SingletonRootInstance;
}

TArray<UGorgeousObjectVariable*> UGorgeousRootObjectVariable::GetVariableHierarchyRegistry()
{
	TArray<TObjectPtr<UGorgeousObjectVariable>> CachedRegistry;
	TArray<TObjectPtr<UGorgeousObjectVariable>> ValidRegistry;

	std::function<void(TObjectPtr<UGorgeousObjectVariable>, TArray<TObjectPtr<UGorgeousObjectVariable>>&)> GatherEntries;
	GatherEntries = [&GatherEntries](const TObjectPtr<UGorgeousObjectVariable>& Variable, TArray<TObjectPtr<UGorgeousObjectVariable>>& Registry)
	{
		if (IsValid(Variable) && !Registry.Contains(Variable))
		{
			Registry.Add(Variable);
			
			for (UGorgeousObjectVariable* Child : Variable->VariableRegistry)
			{
				if (IsValid(Child))
				{
					GatherEntries(Child, Registry);
				}
				else
				{
					UGorgeousLoggingBlueprintFunctionLibrary::LogWarningMessage("Invalid Child TObjectPtr found in VariableRegistry.", "GT.ObjectVariables.Registry.Invalid_Ptr");
				}
			}
		}
	};
	
	for (const TObjectPtr RegistryEntry : RootVariableRegistry)
	{
		GatherEntries(RegistryEntry, CachedRegistry);
	}
	
	for (const TObjectPtr<UGorgeousObjectVariable>& Entry : CachedRegistry)
	{
		if (IsValid(Entry))
		{
			ValidRegistry.Add(Entry);
		}
		else
		{
			UGorgeousLoggingBlueprintFunctionLibrary::LogWarningMessage("Invalid TObjectPtr found in CachedRegistry.", "GT.ObjectVariables.Registry.Invalid_Ptr");
		}
	}

	return ValidRegistry;
}

TArray<UGorgeousObjectVariable*> UGorgeousRootObjectVariable::GetRootVariableRegistry()
{
	return RootVariableRegistry;
}

void UGorgeousRootObjectVariable::RemoveVariableFromRegistry(UGorgeousObjectVariable* VariableToRemove)
{
	if (!VariableToRemove)
	{
		return;
	}

	const std::function RemoveFromRegistry =
		[&](TArray<TObjectPtr<UGorgeousObjectVariable>>& Registry)
		{
			for (int32 i = 0; i < Registry.Num(); ++i)
			{
				if (VariableToRemove && Registry[i] == VariableToRemove)
				{
					Registry.RemoveAt(i);
					if (VariableToRemove->IsRooted())
					{
						VariableToRemove->RemoveFromRoot();
					}
					VariableToRemove->ConditionalBeginDestroy();
					return;
				}
			}
		};
	
	RemoveFromRegistry(RootVariableRegistry);
	
	std::function<void(TArray<TObjectPtr<UGorgeousObjectVariable>>&)> SearchAndRemoveFromChildren =
		[&](TArray<TObjectPtr<UGorgeousObjectVariable>>& Registry)
		{
			for (TObjectPtr<UGorgeousObjectVariable>& Variable : Registry)
			{
				if (IsValid(Variable))
				{
					RemoveFromRegistry(Variable->VariableRegistry);
					SearchAndRemoveFromChildren(Variable->VariableRegistry);
				}
			}
		};

	SearchAndRemoveFromChildren(RootVariableRegistry);
}

bool UGorgeousRootObjectVariable::IsVariableRegistered(UGorgeousObjectVariable* Variable)
{
	return GetVariableHierarchyRegistry().Contains(Variable);
}


void UGorgeousRootObjectVariable::CleanupRegistry(const bool bFullCleanup)
{
	const std::function CleanRegistry =
		[&](TArray<TObjectPtr<UGorgeousObjectVariable>>& Registry)
		{
			for (int32 i = Registry.Num() - 1; i >= 0; --i) // Iterate backwards for safe removal
			{
				if (IsValid(Registry[i]) && (!Registry[i]->bPersistent || bFullCleanup))
				{
					RemoveVariableFromRegistry(Registry[i]);
				}
			}
		};
	
	CleanRegistry(RootVariableRegistry);

	std::function<void(TArray<TObjectPtr<UGorgeousObjectVariable>>&)> CleanChildRegistries =
		[&](TArray<TObjectPtr<UGorgeousObjectVariable>>& Registry)
		{
			for (TObjectPtr<UGorgeousObjectVariable>& Variable : Registry)
			{
				if (IsValid(Variable))
				{
					CleanRegistry(Variable->VariableRegistry);
					CleanChildRegistries(Variable->VariableRegistry); 
				}
			}
		};

	CleanChildRegistries(RootVariableRegistry);

	UGorgeousLoggingBlueprintFunctionLibrary::LogSuccessMessage("Registry cleaned!", "GT.ObjectVariables.Registry.Cleaned");
}

void UGorgeousRootObjectVariable::RegisterWithRegistry(const TObjectPtr<UGorgeousObjectVariable> NewObjectVariable)
{
	if (NewObjectVariable && !IsVariableRegistered(NewObjectVariable))
	{
		RootVariableRegistry.Add(NewObjectVariable);
	}
}
