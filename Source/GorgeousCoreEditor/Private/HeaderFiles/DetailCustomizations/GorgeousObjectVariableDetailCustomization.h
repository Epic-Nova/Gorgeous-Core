/*==========================================================================>
|				Gorgeous Core - Core functionality provider					|
| ------------------------------------------------------------------------- |
|		  Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,			|
|			  administrated by Epic Nova. All rights reserved.				|
| ------------------------------------------------------------------------- |
|					Epic Nova is an independent entity,						|
|	    that is has nothing in common with Epic Games in any capacity.		|
<==========================================================================*/
#pragma once
#include "DetailWidgetRow.h"
#include "IDetailCustomization.h"
#include "ObjectVariables/GorgeousObjectVariableEnums.h"

class FGorgeousObjectVariableDetailCustomization : public IPropertyTypeCustomization
{
public:
	

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
	
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:

	template <typename TEnum>
	TSharedRef<SWidget> GenerateEnumDropdown(FText LabelText, const UEnum* EnumType, TFunction<void(TEnum)> OnSelectionChangedCallback);

};