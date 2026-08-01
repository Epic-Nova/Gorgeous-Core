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
//<--------------------------=== Engine Includes ===------------------------->
#include "IPropertyTypeCustomization.h"
//<-------------------------------------------------------------------------->

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
 * @author Nils Bergemann
 */
class FGorgeousFunctionalStructurePropertyTypeCustomization final : public IPropertyTypeCustomization
{
public:

	/**
	 * Creates an instance of this property type customization.
	 *
	 * @return A shared reference to the newly created instance.
	 */
	GORGEOUSCOREEDITOR_API static TSharedRef<IPropertyTypeCustomization> MakeInstance();

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