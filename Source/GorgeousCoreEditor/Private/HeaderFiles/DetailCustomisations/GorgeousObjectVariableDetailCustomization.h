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
#include "PropertyHandle.h"
//<-------------------------------------------------------------------------->

/**
 * Customizes the detail panel appearance for Gorgeous Object Variable enums,
 * providing a user-friendly dropdown for enum selection.
 *
 * Key features include:
 * - Generation of enum dropdown widgets.
 * - Integration with the Unreal Engine's property system for seamless customization.
 * - Support for both header and child property customization.
 *
 * @note This class enhances the editor experience by providing a visual and intuitive way
 * to select enum values within the details panel.
 */
class FGorgeousObjectVariableDetailCustomization : public IPropertyTypeCustomization
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
     * Generates a dropdown widget for enum selection.
     *
     * @tparam TEnum The enum type.
     * @param LabelText The label text for the dropdown.
     * @param EnumType The UEnum object representing the enum type.
     * @param OnSelectionChangedCallback The callback function to be executed when the selection changes.
     * @return A shared reference to the generated dropdown widget.
     */
    template <typename TEnum>
    TSharedRef<SWidget> GenerateEnumDropdown(FText LabelText, const UEnum* EnumType, TFunction<void(TEnum)> OnSelectionChangedCallback);
};