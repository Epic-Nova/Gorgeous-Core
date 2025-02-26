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
//--------------=== Third Party & Miscellaneous Includes ===----------------->
#include "GorgeousFunctionalStructure.generated.h"
//<-------------------------------------------------------------------------->

/**
 * A structure representing a functional structure.
 *
 * Key features include:
 * - Unique identifier (Identifier).
 * - Default constructor to generate a new identifier.
 * - Virtual destructor.
 * - Editor-specific functions (PostEditChangeProperty, PreEditChangeProperty).
 *
 * @note This structure can be used to define various functional structures within the game.
 */
USTRUCT(Blueprintable, DisplayName = "Gorgeous Functional Structure")
struct GORGEOUSCORERUNTIME_API FGorgeousFunctionalStructure_S
{
	GENERATED_BODY()

	/**
	 * Default constructor. Generates a new unique identifier.
	 */
	FGorgeousFunctionalStructure_S()
	{
		Identifier = FGuid::NewGuid();
	}

	/**
	 * Virtual destructor.
	 */
	virtual ~FGorgeousFunctionalStructure_S() = default;

	/**
	 * Unique identifier for the functional structure.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Functional Structure")
	FGuid Identifier;

#if WITH_EDITOR

	/**
	 * Called after a property is changed in the editor.
	 *
	 * @param PropertyChangedEvent The property changed event.
	 */
	virtual void PostEditChangeProperty(const FPropertyChangedEvent& PropertyChangedEvent) {};

	/**
	 * Called before a property is changed in the editor.
	 *
	 * @param PropertyHandle The property handle.
	 */
	virtual void PreEditChangeProperty(TSharedRef<IPropertyHandle> PropertyHandle) {};
#endif //WITH_EDITOR
};