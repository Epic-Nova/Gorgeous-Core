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

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousCoreRuntimeUtilitiesStructures.h"
//<-------------------------------------------------------------------------->

//<=================--- Delegates ---=================>
DECLARE_MULTICAST_DELEGATE_OneParam(FGorgeousLogEntryDelegate, const FGorgeousLogEntry&);
//<--------------------------------------------------->

//<============================--- C++ Only ---=============================>

/**
 * Retrieves the global Gorgeous log entry delegate.
 *
 * This function provides access to the global delegate used for logging Gorgeous-related entries.
 * It allows other parts of the codebase to bind to this delegate and receive log entries for processing or display.
 *
 * @return A reference to the global FGorgeousLogEntryDelegate instance.
 */
GORGEOUSCORERUNTIMEUTILITIES_API FGorgeousLogEntryDelegate& GetGorgeousLogEntryDelegate();

//<------------------------------------------------------------------------->
