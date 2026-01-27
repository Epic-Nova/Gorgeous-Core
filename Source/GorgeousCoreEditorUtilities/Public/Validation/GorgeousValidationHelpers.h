// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#pragma once

#include "EditorValidatorBase.h"
#include "Settings/GorgeousUsefulSettings.h"

namespace GorgeousValidation
{
	FORCEINLINE bool ShouldTreatWarningsAsErrors()
	{
		const UGorgeousUsefulSettings* Settings = GetDefault<UGorgeousUsefulSettings>();
		return Settings && Settings->bTreatValidationWarningsAsErrors;
	}

	FORCEINLINE bool ReportWarningOrError(UEditorValidatorBase* Validator, const UObject* Asset, const FString& Message)
	{
		if (!Validator)
		{
			return false;
		}

		if (ShouldTreatWarningsAsErrors())
		{
			Validator->AssetFails(Asset, FText::FromString(Message));
			return true;
		}

		Validator->AssetWarning(Asset, FText::FromString(Message));
		return false;
	}
}
