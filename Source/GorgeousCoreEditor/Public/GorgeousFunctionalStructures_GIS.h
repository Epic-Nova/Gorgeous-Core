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
//<-------------------------=== Module Includes ===-------------------------->
#include "FunctionalStructures/GorgeousFunctionalStructure.h"
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousFunctionalStructures_GIS.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Manages the registration and unregistration of functional structures within the editor.
 *
 * Key features include:
 * - RegisterFunctionalStructure: Registers a functional structure for custom property type layout.
 * - UnregisterFunctionalStructure: Unregisters a functional structure.
 *
 * @note This subsystem provides a centralized way to handle functional structure registrations.
 */
UCLASS(BlueprintType, DisplayName = "Gorgeous Functional Structures")
class UGorgeousFunctionalStructures_GIS : public UEditorSubsystem
{
	GENERATED_BODY()

public:

	/**
	 * Registers a functional structure for custom property type layout.
	 *
	 * @param Structure The functional structure to register.
	 */
	UFUNCTION(BlueprintCallable)
	void RegisterFunctionalStructure(UPARAM(ref) FGorgeousFunctionalStructure_S& Structure);

	/**
	 * Unregisters a functional structure.
	 *
	 * @param Structure The functional structure to unregister.
	 */
	UFUNCTION(BlueprintCallable)
	void UnregisterFunctionalStructure(UPARAM(ref) FGorgeousFunctionalStructure_S& Structure);
};