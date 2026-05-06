// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "DetailCustomizations/GorgeousGlobalDetailCustomization.h"
#include "DetailExtensions/GorgeousDetailExtension.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "PropertyHandle.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UObjectIterator.h"

TMap<FName, TStrongObjectPtr<UGorgeousDetailExtension>> FGorgeousGlobalDetailCustomization::ExtensionRegistry;

TSharedRef<IDetailCustomization> FGorgeousGlobalDetailCustomization::MakeInstance()
{
	return MakeShareable(new FGorgeousGlobalDetailCustomization);
}

void FGorgeousGlobalDetailCustomization::RegisterExtension(UGorgeousDetailExtension* Extension)
{
	if (!Extension) return;
	ExtensionRegistry.Add(Extension->GetExtensionName(), TStrongObjectPtr<UGorgeousDetailExtension>(Extension));
}

void FGorgeousGlobalDetailCustomization::UnregisterExtension(FName ExtensionName)
{
	ExtensionRegistry.Remove(ExtensionName);
}

void FGorgeousGlobalDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	ProcessProperties(DetailBuilder);
}

void FGorgeousGlobalDetailCustomization::ProcessProperties(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);

	if (Objects.Num() == 0) return;

	UClass* ObjectClass = Objects[0]->GetClass();
	if (!ObjectClass) return;

	static const FName MetaDataTag = TEXT("GorgeousDetailExtension");

	// Iterate through all properties of the class
	for (TFieldIterator<FProperty> It(ObjectClass); It; ++It)
	{
		FProperty* Property = *It;
		if (Property->HasMetaData(MetaDataTag))
		{
			FString ExtensionNameStr = Property->GetMetaData(MetaDataTag);
			FName ExtensionName = *ExtensionNameStr;

			if (TStrongObjectPtr<UGorgeousDetailExtension>* ExtensionPtr = ExtensionRegistry.Find(ExtensionName))
			{
				UGorgeousDetailExtension* Extension = ExtensionPtr->Get();
				TSharedRef<IPropertyHandle> PropertyHandle = DetailBuilder.GetProperty(Property->GetFName());
				
				if (PropertyHandle->IsValidHandle())
				{
					// Get the category this property belongs to
					FName CategoryName = Property->GetMetaData(TEXT("Category")).IsEmpty() ? TEXT("Default") : *Property->GetMetaData(TEXT("Category"));
					IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(CategoryName);
					
					// Add the property row and let the extension customize it
					IDetailPropertyRow& PropertyRow = Category.AddProperty(PropertyHandle);
					
					FDetailWidgetRow& HeaderRow = PropertyRow.CustomWidget(false);
					Extension->CustomizeHeader(PropertyHandle, HeaderRow);
				}
			}
		}
	}
}
