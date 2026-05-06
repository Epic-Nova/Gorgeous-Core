// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#include "GorgeousFunctionalStructurePropertyTypeCustomization.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousCoreUtilitiesMinimalShared.h"
#include "FunctionalStructures/GorgeousFunctionalStructure.h"
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
	/* @TODO: fix the crash issue*/
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
		
		// PREVENT INFINITE RECURSION: Do not draw the OwnerObject pointer.
		// If drawn, the property editor will recursively evaluate the Outer object and loop forever.
		if (ChildHandle->GetProperty()->GetFName() == FName("OwnerObject"))
		{
			continue;
		}
		
		if (ChildHandle->GetBoolMetaData(TEXT("ShowOnlyInnerProperties"))) // Promote inner properties into this level
		{
			uint32 InnerNumChildren = 0;
			ChildHandle->GetNumChildren(InnerNumChildren);
			for (uint32 j = 0; j < InnerNumChildren; j++)
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
		GORGEOUS_55_HIGHER(
			IDetailGroup& AdvancedGroup = ChildBuilder.AddGroup("Advanced", FText::FromString("Advanced"), false);
		)
		GORGEOUS_54_LOWER(
			IDetailGroup& AdvancedGroup = ChildBuilder.AddGroup("Advanced", FText::FromString("Advanced"));
		)
		for (const auto AdvancedHandle : AdvancedHandles)
		{
			AdvancedGroup.AddPropertyRow(AdvancedHandle);
		}
		
		// Delay the collapse slightly because when we don't do this, we are greeted with an expanded category
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
				GT_I_LOG("GT.FunctionalStructures.Struct_Instance_Received", TEXT("Gorgeous Struct Instance received for: %s"), *FunctionalStructureInstance->Identifier.ToString());

				TArray<UObject*> OuterObjects;
				PropertyHandle->GetOuterObjects(OuterObjects);
				if (OuterObjects.Num() > 0)
				{
					if (FunctionalStructureInstance->OwnerObject != OuterObjects[0])
					{
						FunctionalStructureInstance->OwnerObject = OuterObjects[0];
						FunctionalStructureInstance->AllocateDefaultValues(FunctionalStructureInstance->OwnerObject);
					}
				}
			}
		}
	}
}

void FGorgeousFunctionalStructurePropertyTypeCustomization::OnPreChildPropertyChanged(TSharedRef<IPropertyHandle> PropertyHandle) const
{
	if (!PropertyHandle->IsValidHandle() || !PropertyHandle->GetProperty() || !FunctionalStructureInstance)
	{
		GT_E_LOG("GT.FunctionalStructures.PreEditChangeProperty.Invalid", TEXT("Invalid property handle or struct instance in PreEditChangeProperty callback"));
		return;
	}
	
	FunctionalStructureInstance->PreEditChangeProperty(PropertyHandle);
	GT_S_LOG("GT.FunctionalStructures.PreEditChangeProperty.Called", TEXT("PreEditChangeProperty called for '%s'"), *FunctionalStructureInstance->Identifier.ToString());
}

void FGorgeousFunctionalStructurePropertyTypeCustomization::OnChildPropertyChangedWithData(const FPropertyChangedEvent& PropertyChangedEvent) const
{
	if (!PropertyChangedEvent.Property || !FunctionalStructureInstance)
	{
		GT_E_LOG("GT.FunctionalStructures.PostEditChangeProperty.Invalid", TEXT("Invalid property or struct instance in PostEditChangeProperty callback"));
		return;
	}
	
	FunctionalStructureInstance->PostEditChangeProperty(PropertyChangedEvent);
	GT_S_LOG("GT.FunctionalStructures.PostEditChangeProperty.Called", TEXT("PostEditChangeProperty called for '%s'"), *FunctionalStructureInstance->Identifier.ToString());
}