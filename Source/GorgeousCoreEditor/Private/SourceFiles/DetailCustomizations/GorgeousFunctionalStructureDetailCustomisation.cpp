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
#include "GorgeousFunctionalStructureDetailCustomisation.h"

//<=============================--- Includes ---=============================>
//<-------------------------=== Engine Includes ===-------------------------->
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#include "PropertyEditorModule.h"
#include "IDetailChildrenBuilder.h"
//<-------------------------=== Module Includes ===-------------------------->
#include "GorgeousCoreUtilitiesMinimalShared.h"
#include "FunctionalStructures/GorgeousFunctionalStructure.h"
//<-------------------------------------------------------------------------->


//=============================================================================
// FGorgeousFunctionalStructureDetailCustomisation Implementation
//=============================================================================

TSharedRef<IPropertyTypeCustomization> FGorgeousFunctionalStructureDetailCustomisation::MakeInstance()
{
	return MakeShareable(new FGorgeousFunctionalStructureDetailCustomisation);
}

void FGorgeousFunctionalStructureDetailCustomisation::CustomizeHeader(const TSharedRef<IPropertyHandle> PropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	HeaderRow.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	];
}

void FGorgeousFunctionalStructureDetailCustomisation::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle,
	IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	//Binds a Callback to every property. When changed call PostEditChangeProperty on this struct or when about to be changed call PreEditChangeProperty
	uint32 NumChildren;
	PropertyHandle->GetNumChildren(NumChildren);

	for (uint32 i = 0; i < NumChildren; i++)
	{
		const TSharedRef<IPropertyHandle> ChildHandle = PropertyHandle->GetChildHandle(i).ToSharedRef();
		ChildBuilder.AddProperty(ChildHandle);

		TDelegate<void(const FPropertyChangedEvent&)> PropertyChangedDelegate =	
			TDelegate<void(const FPropertyChangedEvent&)>::CreateSP(this, &FGorgeousFunctionalStructureDetailCustomisation::OnChildPropertyChangedWithData);
		ChildHandle->SetOnPropertyValueChangedWithData(PropertyChangedDelegate);
		ChildHandle->SetOnChildPropertyValueChangedWithData(PropertyChangedDelegate);

		FSimpleDelegate PrePropertyChangedDelegate = FSimpleDelegate::CreateSP(this, &FGorgeousFunctionalStructureDetailCustomisation::OnPreChildPropertyChanged, PropertyHandle);
		ChildHandle->SetOnPropertyValuePreChange(PrePropertyChangedDelegate);
		ChildHandle->SetOnChildPropertyValuePreChange(PrePropertyChangedDelegate);
	}

	//Receive the struct instance from via the property handle
	if (CastField<FStructProperty>(PropertyHandle->GetProperty()))
	{
		void* StructInstance = nullptr;
		PropertyHandle->GetValueData(StructInstance);

		if (StructInstance)
		{
			FunctionalStructureInstance = static_cast<FGorgeousFunctionalStructure_S*>(StructInstance);
			if (FunctionalStructureInstance)
			{
				UGorgeousLoggingBlueprintFunctionLibrary::LogInformationMessage(FString::Printf(TEXT("Gorgeous Struct Instance received for: %s"),
					*FunctionalStructureInstance->Identifier.ToString()), "GT.FunctionalStructures.Struct_Instance_Received", 2.f, false, true);
			}
		}
	}
}

void FGorgeousFunctionalStructureDetailCustomisation::OnPreChildPropertyChanged(TSharedRef<IPropertyHandle> PropertyHandle) const
{
	if (PropertyHandle->IsValidHandle() && PropertyHandle->GetProperty())
	{
		UGorgeousLoggingBlueprintFunctionLibrary::LogInformationMessage(FString::Printf(TEXT("Property is about to be '%s' changed!"),
			*PropertyHandle->GetProperty()->GetFName().ToString()), "GT.FunctionalStructures.PreEditChangeProperty.Queued", 2.f, false, true);

		if (FunctionalStructureInstance)
		{
			FunctionalStructureInstance->PreEditChangeProperty(PropertyHandle);
			UGorgeousLoggingBlueprintFunctionLibrary::LogSuccessMessage(FString::Printf(TEXT("PreEditChangeProperty called for '%s'"),
				*FunctionalStructureInstance->Identifier.ToString()), "GT.FunctionalStructures.PreEditChangeProperty.Called", false, true);
		}
	}
}

void FGorgeousFunctionalStructureDetailCustomisation::OnChildPropertyChangedWithData(
	const FPropertyChangedEvent& PropertyChangedEvent) const
{
	if (PropertyChangedEvent.Property)
	{
		UGorgeousLoggingBlueprintFunctionLibrary::LogInformationMessage(FString::Printf(TEXT("Property: '%s' changed!"),
			*PropertyChangedEvent.Property->GetFName().ToString()), "GT.FunctionalStructures.PostEditChangeProperty.Queued", 2.f, false, true);

		if (FunctionalStructureInstance)
		{
			FunctionalStructureInstance->PostEditChangeProperty(PropertyChangedEvent);
			UGorgeousLoggingBlueprintFunctionLibrary::LogSuccessMessage(FString::Printf(TEXT("PostEditChangeProperty called for '%s'"),
				*FunctionalStructureInstance->Identifier.ToString()), "GT.FunctionalStructures.PostEditChangeProperty.Called", false, true);
		}
	}
}
