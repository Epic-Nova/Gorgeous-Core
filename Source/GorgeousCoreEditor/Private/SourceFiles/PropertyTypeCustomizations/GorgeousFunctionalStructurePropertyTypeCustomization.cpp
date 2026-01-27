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
#include "GorgeousFunctionalStructurePropertyTypeCustomization.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#include "PropertyEditorModule.h"
#include "IDetailChildrenBuilder.h"
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousCoreUtilitiesMinimalShared.h"
#include "IDetailGroup.h"
#include "FunctionalStructures/GorgeousFunctionalStructure.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// FGorgeousFunctionalStructureDetailCustomisation Implementation
//=============================================================================

TSharedRef<IPropertyTypeCustomization> FGorgeousFunctionalStructurePropertyTypeCustomization::MakeInstance()
{
	return MakeShareable(new FGorgeousFunctionalStructurePropertyTypeCustomization);
}

void FGorgeousFunctionalStructurePropertyTypeCustomization::CustomizeHeader(const TSharedRef<IPropertyHandle> PropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	HeaderRow.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	];
}

void FGorgeousFunctionalStructurePropertyTypeCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle,
	IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	//@TODO: Maybe add the case of when SimpleDisplay is added to a property, where its parent property has AdvancedDisplay that the Property with SimpleDisplay gets its simple display form. => At this point it would just be a flex
	
	TArray<TSharedRef<IPropertyHandle>> AdvancedHandles;
	
	auto AddToGroup = [&](const TSharedRef<IPropertyHandle>& Handle)
	{
		const bool bNeedsAdvancedDisplay = Handle->GetParentHandle() && Handle->GetParentHandle()->HasMetaData("ShowOnlyInnerProperties") 
		? Handle->GetParentHandle()->GetProperty()->HasAnyPropertyFlags(CPF_AdvancedDisplay) 
		: Handle->GetProperty()->HasAnyPropertyFlags(CPF_AdvancedDisplay);

		if (bNeedsAdvancedDisplay)
		{
			AdvancedHandles.Add(Handle);
			return;
		}
		ChildBuilder.AddProperty(Handle);
	};
	
	uint32 NumChildren;
	PropertyHandle->GetNumChildren(NumChildren);
	
	//Binds a Callback to every property. When changed call PostEditChangeProperty on this struct or when about to be changed call PreEditChangeProperty
	for (uint32 i = 0; i < NumChildren; i++)
	{
		const TSharedRef<IPropertyHandle> ChildHandle = PropertyHandle->GetChildHandle(i).ToSharedRef();
		
		if (ChildHandle->GetBoolMetaData(TEXT("ShowOnlyInnerProperties"))) // Promote inner properties into this level
		{
			ChildHandle->GetNumChildren(NumChildren);
			for (uint32 j = 0; j < NumChildren; j++)
			{
				const TSharedRef<IPropertyHandle> GrandChildHandle = ChildHandle->GetChildHandle(j).ToSharedRef();
				AddToGroup(GrandChildHandle);
			}
		}
		else
		{
			AddToGroup(ChildHandle);
		}
		
		TDelegate<void(const FPropertyChangedEvent&)> PropertyChangedDelegate =	
			TDelegate<void(const FPropertyChangedEvent&)>::CreateSP(this, &FGorgeousFunctionalStructurePropertyTypeCustomization::OnChildPropertyChangedWithData);
		ChildHandle->SetOnPropertyValueChangedWithData(PropertyChangedDelegate);
		ChildHandle->SetOnChildPropertyValueChangedWithData(PropertyChangedDelegate);

		FSimpleDelegate PrePropertyChangedDelegate = FSimpleDelegate::CreateSP(this, &FGorgeousFunctionalStructurePropertyTypeCustomization::OnPreChildPropertyChanged, PropertyHandle);
		ChildHandle->SetOnPropertyValuePreChange(PrePropertyChangedDelegate);
		ChildHandle->SetOnChildPropertyValuePreChange(PrePropertyChangedDelegate);
	}
	
	if (!AdvancedHandles.IsEmpty())
	{
		IDetailGroup& AdvancedGroup = ChildBuilder.AddGroup("Advanced", FText::FromString("Advanced"), false);
		for (const auto AdvancedHandle : AdvancedHandles)
		{
			AdvancedGroup.AddPropertyRow(AdvancedHandle);
		}
		
		// Delay the collapse slightly because when we don't do this we are greeted with an expanded category
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([&AdvancedGroup](float)
		{
			AdvancedGroup.ToggleExpansion(false);
			return false; // remove ticker
		}));
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
				GT_I_LOG_FULL(TEXT("Gorgeous Struct Instance received for: %s"),
					"GT.FunctionalStructures.Struct_Instance_Received",
					2.f,
					false,
					true,
					nullptr,
					*FunctionalStructureInstance->Identifier.ToString());

				TArray<UObject*> OuterObjects;
				PropertyHandle->GetOuterObjects(OuterObjects);
				if (OuterObjects.Num() > 0)
				{
					FunctionalStructureInstance->OwnerObject = OuterObjects[0];
				}

				FunctionalStructureInstance->AllocateDefaultValues(FunctionalStructureInstance->OwnerObject);
			}
		}
	}
}

void FGorgeousFunctionalStructurePropertyTypeCustomization::OnPreChildPropertyChanged(TSharedRef<IPropertyHandle> PropertyHandle) const
{
	if (PropertyHandle->IsValidHandle() && PropertyHandle->GetProperty())
	{
		GT_I_LOG_FULL(TEXT("Property: '%s' is about to be changed!"),
			"GT.FunctionalStructures.PreEditChangeProperty.Queued",
			2.f,
			false,
			true,
			nullptr,
			*PropertyHandle->GetProperty()->GetFName().ToString());

		if (FunctionalStructureInstance)
		{
			FunctionalStructureInstance->PreEditChangeProperty(PropertyHandle);
			GT_S_LOG_FULL(TEXT("PreEditChangeProperty called for '%s'"),
				"GT.FunctionalStructures.PreEditChangeProperty.Called",
				0.0f,
				true,
				true,
				nullptr,
				*FunctionalStructureInstance->Identifier.ToString());
		}
	}
}

void FGorgeousFunctionalStructurePropertyTypeCustomization::OnChildPropertyChangedWithData(
	const FPropertyChangedEvent& PropertyChangedEvent) const
{
	if (PropertyChangedEvent.Property)
	{
		GT_I_LOG_FULL(TEXT("Property: '%s' changed!"),
			"GT.FunctionalStructures.PostEditChangeProperty.Queued",
			2.f,
			false,
			true,
			nullptr,
			*PropertyChangedEvent.Property->GetFName().ToString());

		if (FunctionalStructureInstance)
		{
			FunctionalStructureInstance->PostEditChangeProperty(PropertyChangedEvent);
			GT_S_LOG_FULL(TEXT("PostEditChangeProperty called for '%s'"),
				"GT.FunctionalStructures.PostEditChangeProperty.Called",
				0.0f,
				true,
				true,
				nullptr,
				*FunctionalStructureInstance->Identifier.ToString());
		}
	}
}
