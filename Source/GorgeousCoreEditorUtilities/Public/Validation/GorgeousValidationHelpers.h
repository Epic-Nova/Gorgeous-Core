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

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "EditorValidatorBase.h"
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousUsefulSettings.h"
//<-------------------------------------------------------------------------->

namespace GorgeousValidation
{
	// Helper function to determine if validation warnings should be treated as errors based on user settings.
	FORCEINLINE bool ShouldTreatWarningsAsErrors()
	{
		const UGorgeousUsefulSettings* Settings = GetDefault<UGorgeousUsefulSettings>();
		return Settings && Settings->bTreatValidationWarningsAsErrors;
	}

	/**
	 * Reports a validation issue as either a warning or an error based on user settings.
	 *
	 * @param Validator - The validator instance to report to.
	 * @param Asset - The asset that is being validated.
	 * @param Message - The message describing the validation issue.
	 * @return true if the issue was reported as an error, false if it was reported as a warning or if the validator is null.
	 */
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
