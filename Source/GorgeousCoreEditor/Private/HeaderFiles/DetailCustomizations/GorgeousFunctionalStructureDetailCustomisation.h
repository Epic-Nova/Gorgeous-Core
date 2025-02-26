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
#include "IPropertyTypeCustomization.h"

struct FGorgeousFunctionalStructure_S;

class FGorgeousFunctionalStructureDetailCustomisation final : public IPropertyTypeCustomization
{
public:
	
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
	
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:

	void OnPreChildPropertyChanged(TSharedRef<IPropertyHandle> PropertyHandle) const;
	
	void OnChildPropertyChangedWithData(const FPropertyChangedEvent& PropertyChangedEvent) const;
	
	FGorgeousFunctionalStructure_S* FunctionalStructureInstance = nullptr;
};