// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#include "ConditionalObjectChoosers/GorgeousConditionalObjectChooser.h"

//=============================================================================
// Helpers
//=============================================================================

namespace
{
	/**
	 * Best-effort placeholder detection without depending on private engine headers.
	 * Linker placeholders usually carry PLACEHOLDER in their name or outer name.
	 */
	bool IsLikelyPlaceholder(const UObject* Obj)
	{
		if (!IsValid(Obj))
		{
			return false;
		}

		const FString ObjName = Obj->GetName();
		if (ObjName.Contains(TEXT("PLACEHOLDER")))
		{
			return true;
		}

		if (const UObject* Outer = Obj->GetOuter())
		{
			const FString OuterName = Outer->GetName();
			if (OuterName.Contains(TEXT("PLACEHOLDER")))
			{
				return true;
			}
		}

		return false;
	}
}

//=============================================================================
// UGorgeousConditionalObjectChooser Implementation
//=============================================================================

void UGorgeousConditionalObjectChooser::CleanupInvalidEntries()
{
	// Clean ConditionCheck first
	if (IsLikelyPlaceholder(ConditionCheck) || !IsValid(ConditionCheck) || !ConditionCheck->IsA(UGorgeousCondition::StaticClass()))
	{
		ConditionCheck = nullptr;
	}

	// Clean Conditions array
	Conditions.RemoveAll([](const TObjectPtr<UGorgeousObjectVariable>& Entry)
	{
		return !IsValid(Entry) || IsLikelyPlaceholder(Entry.Get());
	});
}

UGorgeousObjectVariable* UGorgeousConditionalObjectChooser::DecideCondition() const
{
	if (!IsValid(ConditionCheck) || Conditions.Num() == 0)
	{
		return nullptr;
	}

	const int32 ConditionIndex = static_cast<int32>(ConditionCheck->CheckCondition());
	if (!Conditions.IsValidIndex(ConditionIndex))
	{
		return nullptr;
	}

	if (UGorgeousObjectVariable* ObjectVariable = Conditions[ConditionIndex])
	{
		ObjectVariable->InvokeInstancedFunctionality(FGuid::NewGuid());
		return ObjectVariable;
	}

	return nullptr;
}

void UGorgeousConditionalObjectChooser::PostLoad()
{
	Super::PostLoad();

	// Drop unresolved placeholder objects that can sneak in when blueprint dependencies circle back to this class.
	CleanupInvalidEntries();
	SetConditionCheck(ConditionCheck);
}

void UGorgeousConditionalObjectChooser::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (Ar.IsLoading())
	{
		// Strip placeholders before dependency repair runs on the loaded object.
		CleanupInvalidEntries();
	}
}

void UGorgeousConditionalObjectChooser::SetConditionCheck(UGorgeousCondition* NewConditionCheck)
{
	// Reject invalid or unrelated types up front.
	if (!IsValid(NewConditionCheck) || !NewConditionCheck->IsA(UGorgeousCondition::StaticClass()))
	{
		ConditionCheck = nullptr;
		return;
	}

	// If the object is an archetype/placeholder, create a safe instanced copy owned by this chooser.
	if (NewConditionCheck->IsTemplate() || IsLikelyPlaceholder(NewConditionCheck) || NewConditionCheck->GetOuter() != this)
	{
		ConditionCheck = DuplicateObject<UGorgeousCondition>(NewConditionCheck, this);
		return;
	}

	ConditionCheck = NewConditionCheck;
}