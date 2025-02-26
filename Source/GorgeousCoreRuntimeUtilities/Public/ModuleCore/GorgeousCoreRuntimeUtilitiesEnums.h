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

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousCoreRuntimeUtilitiesEnums.generated.h"
//<-------------------------------------------------------------------------->

/**
 * @brief Enum that defines the importancy of the logging message.
 * @author Nils Bergemann
 * @since Beginning
 */
UENUM(BlueprintType, DisplayName = "Gorgeous Logging Importancy", meta = (ShortTooltip = "The importancy of the logging message."))
enum EGorgeousLoggingImportancy
{
	Logging_Information = 0 UMETA(DisplayName = "Information", ToolTip = "Informational message."),
	Logging_Success = 1 UMETA(DisplayName = "Success", ToolTip = "Success message."),
	Logging_Warning = 2 UMETA(DisplayName = "Warning", ToolTip = "Warning message."),
	Logging_Error = 3 UMETA(DisplayName = "Error", ToolTip = "Error message."),
	Logging_Fatal = 4 UMETA(DisplayName = "Fatal", ToolTip = "Fatal error message."),
	Logging_Max = 5 UMETA(Hidden)
};
