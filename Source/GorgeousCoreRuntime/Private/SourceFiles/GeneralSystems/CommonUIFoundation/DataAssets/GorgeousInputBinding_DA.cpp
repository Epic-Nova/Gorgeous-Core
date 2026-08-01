// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousInputBinding_DA.h"
#include "InputAction.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"

EDataValidationResult UGorgeousInputBinding_DA::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	TSet<UInputAction*> AssignedActions;
	for (const auto& Pair : Bindings)
	{
		if (!Pair.Key.IsValid())
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("Input Binding in %s has an invalid Gameplay Tag."), *GetName())));
			Result = EDataValidationResult::Invalid;
		}

		UInputAction* Action = Pair.Value.Action;
		if (!Action)
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("Input Binding for tag '%s' in %s has no Input Action assigned."), *Pair.Key.ToString(), *GetName())));
			Result = EDataValidationResult::Invalid;
			continue;
		}

		if (AssignedActions.Contains(Action))
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("Input Action '%s' is assigned multiple times in %s. This will cause input conflicts."), *Action->GetName(), *GetName())));
			Result = EDataValidationResult::Invalid;
		}
		AssignedActions.Add(Action);
	}

	return Result;
}
#endif
