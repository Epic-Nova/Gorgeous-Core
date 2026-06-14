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
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousFunctionalStructure.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Structure for defining functional structures within the game. 
 * This structure can be extended to include various properties and functions as needed for specific use cases.
 * 
 * Functional structures enable structures to receive editor events and have an owner object, 
 * allowing for more dynamic and interactive data structures in Unreal Engine.
 * 
 * @author Nils Bergemann
 */
USTRUCT(Blueprintable, DisplayName = "Gorgeous Functional Structure")
struct GORGEOUSCORERUNTIME_API FGorgeousFunctionalStructure_S
{
	GENERATED_BODY()

	//<================--- Friend Classes ---================>
#if WITH_EDITOR
	friend class FGorgeousFunctionalStructurePropertyTypeCustomization;
#endif
	//<------------------------------------------------------>

	// Default constructor that initializes the identifier with a new GUID and sets the owner object to nullptr.
	FGorgeousFunctionalStructure_S()
	{
		Identifier = FGuid::NewGuid();
		OwnerObject = nullptr;
	}

	/** 
	 * The destructor is declared as virtual to allow for proper cleanup in case of inheritance, 
	 * even though USTRUCTs typically do not have virtual functions. 
	 * This is a safeguard for any potential future extensions where this struct might be inherited by a class or another struct that requires a destructor.
	 */
	virtual ~FGorgeousFunctionalStructure_S() = default;

	// Unique identifier for this functional structure instance. This can be used to track instances across editor sessions or for debugging purposes.
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
	 * The owner object of this functional structure. 
	 * This allows the structure to have context about which UObject it belongs to, 
	 * enabling it to respond to editor events and interact with the owning object as needed.
	 */
	UPROPERTY(Transient)
	UObject* OwnerObject;

#endif
};