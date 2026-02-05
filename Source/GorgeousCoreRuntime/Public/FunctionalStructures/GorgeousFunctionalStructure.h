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

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//----------------=== Third Party & Miscellaneous Includes ===--------------->
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

	//<================--- Friend Classes ---================>
	friend class FGorgeousFunctionalStructurePropertyTypeCustomization;
	//<------------------------------------------------------>

	/**
	 * Default constructor.
	 */
	FGorgeousFunctionalStructure_S()
	{
		Identifier = FGuid::NewGuid();
		OwnerObject = nullptr;
	}


	/**
	 * Virtual destructor.
	 */
	virtual ~FGorgeousFunctionalStructure_S() = default;

	/**
	 * Unique identifier for the functional structure.
	 * Initialized to empty GUID to satisfy UHT requirements, then set to new GUID in constructor.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Functional Structure", meta = (DisplayPriority = 99))
	FGuid Identifier = FGuid();

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

	/**
	 * Gives the chance for default value allocation after initialisation.
	 * 
	 * @param OuterOwner The owner UObject of this structure.
	 */
	virtual void AllocateDefaultValues(UObject* OuterOwner) {};
	
#endif //WITH_EDITOR

protected:

#if WITH_EDITORONLY_DATA
	
	/**
	 * The outer object that holds this structure. Can be used for Instanced UPROPERTY's.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Functional Structure", meta = (DisplayPriority = 99))
	UObject* OwnerObject;

#endif WITH_EDITORONLY_DATA
};