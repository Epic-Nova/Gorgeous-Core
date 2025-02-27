// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<-------------------------=== Engine Includes ===-------------------------->
#include "IPropertyTypeCustomization.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
struct FGorgeousFunctionalStructure_S;
//<-------------------------------------------------->

/**
 * Property type customization for functional structures, enabling them to receive
 * PostEditChangeProperty and PreEditChangeProperty events within the Unreal Engine 5 editor's
 * details panel. This class facilitates a more interactive and responsive editing experience
 * for custom data structures, allowing developers to implement real-time updates and validations
 * as properties are modified.
 *
 * This customization is particularly useful for structures that require complex logic or
 * dependencies between their properties, as it provides a robust mechanism to handle
 * property changes and trigger corresponding actions.
 *
 * Key features include:
 * - Integration with the Unreal Engine's property system for seamless customization.
 * - Support for both header and child property customization.
 * - Event-driven updates through Pre- and Post-edit change events.
 * - A dedicated instance pointer to the functional structure being customized, enabling direct access and modification.
 *
 * @author Nils Bergemann
 * @since Beginning
 * @version v1.0
 */
class FGorgeousFunctionalStructureDetailCustomisation final : public IPropertyTypeCustomization
{
public:

	/**
	 * Creates an instance of this property type customization.
	 *
	 * @return A shared reference to the newly created instance.
	 */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/**
	 * Customizes the header row of the property in the details panel.
	 *
	 * @param PropertyHandle The property handle being customized.
	 * @param HeaderRow The detail widget row for the header.
	 * @param CustomizationUtils Utility functions for property customization.
	 */
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	/**
	 * Customizes the children of the property in the details panel.
	 *
	 * @param PropertyHandle The property handle being customized.
	 * @param ChildBuilder The detail children builder.
	 * @param CustomizationUtils Utility functions for property customization.
	 */
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:

	/**
	 * Called before a child property is changed.
	 *
	 * @param PropertyHandle The property handle of the child being changed.
	 */
	void OnPreChildPropertyChanged(TSharedRef<IPropertyHandle> PropertyHandle) const;

	/**
	 * Called after a child property is changed, with detailed event information.
	 *
	 * @param PropertyChangedEvent The event containing details about the property change.
	 */
	void OnChildPropertyChangedWithData(const FPropertyChangedEvent& PropertyChangedEvent) const;

	/**
	 * Pointer to the functional structure instance being customized.
	 *
	 * @note This pointer is used to access and modify the functional structure's properties.
	 */
	FGorgeousFunctionalStructure_S* FunctionalStructureInstance = nullptr;
};