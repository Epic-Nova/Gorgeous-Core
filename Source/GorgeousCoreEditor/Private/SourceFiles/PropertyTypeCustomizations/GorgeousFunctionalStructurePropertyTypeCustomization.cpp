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
	FunctionalPropertyHandle = PropertyHandle;
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
		TSharedPtr<IPropertyHandle> Child = PropertyHandle->GetChildHandle(i);
		if (!Child.IsValid() || !Child->IsValidHandle() || !Child->GetProperty()) continue;
		const TSharedRef<IPropertyHandle> ChildHandle = Child.ToSharedRef();
		
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
				if (TSharedPtr<IPropertyHandle> GrandChild = ChildHandle->GetChildHandle(j); GrandChild.IsValid() && GrandChild->IsValidHandle() && GrandChild->GetProperty()) AddToGroup(GrandChild.ToSharedRef());
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
		for (const auto& AdvancedHandle : AdvancedHandles)
		{
			AdvancedGroup.AddPropertyRow(AdvancedHandle);
		}
		

	}
	
	TArray<UObject*> Owners;
	PropertyHandle->GetOuterObjects(Owners);
	int32 OwnerIndex = 0;
	ForEachInstance([&](FGorgeousFunctionalStructure_S& Instance)
	{
		UObject* Owner = Owners.IsValidIndex(OwnerIndex) ? Owners[OwnerIndex] : (Owners.IsEmpty() ? nullptr : Owners[0]);
		++OwnerIndex;
		if (Owner && Instance.OwnerObject != Owner)
		{
			Instance.OwnerObject = Owner;
			Instance.AllocateDefaultValues(Owner);
		}
	});
}

void FGorgeousFunctionalStructurePropertyTypeCustomization::ForEachInstance(TFunctionRef<void(FGorgeousFunctionalStructure_S&)> Callback) const
{
	TSharedPtr<IPropertyHandle> Handle = FunctionalPropertyHandle.Pin();
	if (!Handle.IsValid() || !Handle->IsValidHandle()) return;
	const FStructProperty* Property = CastField<FStructProperty>(Handle->GetProperty());
	if (!Property || !Property->Struct->IsChildOf(FGorgeousFunctionalStructure_S::StaticStruct())) return;
	TArray<void*> Instances;
	Handle->AccessRawData(Instances);
	for (void* Instance : Instances) if (Instance) Callback(*static_cast<FGorgeousFunctionalStructure_S*>(Instance));
}

void FGorgeousFunctionalStructurePropertyTypeCustomization::OnPreChildPropertyChanged(TSharedRef<IPropertyHandle> PropertyHandle) const
{
	ForEachInstance([&](FGorgeousFunctionalStructure_S& Instance) { Instance.PreEditChangeProperty(PropertyHandle); });
}

void FGorgeousFunctionalStructurePropertyTypeCustomization::OnChildPropertyChangedWithData(const FPropertyChangedEvent& PropertyChangedEvent) const
{
	ForEachInstance([&](FGorgeousFunctionalStructure_S& Instance) { Instance.PostEditChangeProperty(PropertyChangedEvent); });
}
