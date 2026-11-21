// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once

#include "CoreMinimal.h"

//<=============================--- Includes ---=============================>
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousCoreRuntimeUtilitiesEnums.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Enum that defines the importance of the logging message.
 * @author Nils Bergemann
 */
UENUM(BlueprintType, meta = (DisplayName = "Gorgeous Logging Importance", ShortTooltip = "The importance of the logging message."))
enum EGorgeousLoggingImportance
{
	Logging_Information = 0 UMETA(DisplayName = "Information", ToolTip = "Informational message."),
	Logging_Success = 1 UMETA(DisplayName = "Success", ToolTip = "Success message."),
	Logging_Warning = 2 UMETA(DisplayName = "Warning", ToolTip = "Warning message."),
	Logging_Error = 3 UMETA(DisplayName = "Error", ToolTip = "Error message."),
	Logging_Fatal = 4 UMETA(DisplayName = "Fatal", ToolTip = "Fatal error message."),
	Logging_Verbose = 5 UMETA(DisplayName = "Verbose", ToolTip = "Verbose diagnostic message."),
	Logging_VeryVerbose = 6 UMETA(DisplayName = "Very Verbose", ToolTip = "Detailed diagnostic trace."),
	Logging_Display = 7 UMETA(DisplayName = "Display", ToolTip = "Visible informational message without success semantics."),
	Logging_MAX = 8 UMETA(Hidden)
};