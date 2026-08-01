#include "DetailCustomizations/GorgeousGlobalDetailCustomization.h"
#include "DetailExtensions/GorgeousDetailExtension.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "PropertyHandle.h"
#include "UObject/UObjectIterator.h"

TMap<FName, TStrongObjectPtr<UGorgeousDetailExtension>> FGorgeousGlobalDetailCustomization::ExtensionRegistry;

TSharedRef<IDetailCustomization> FGorgeousGlobalDetailCustomization::MakeInstance() { return MakeShareable(new FGorgeousGlobalDetailCustomization); }

void FGorgeousGlobalDetailCustomization::RegisterExtension(UGorgeousDetailExtension* Extension)
{
    if (!Extension) return;
    ExtensionRegistry.Add(Extension->GetExtensionName(), TStrongObjectPtr<UGorgeousDetailExtension>(Extension));
}

void FGorgeousGlobalDetailCustomization::UnregisterExtension(FName ExtensionName) { ExtensionRegistry.Remove(ExtensionName); }
void FGorgeousGlobalDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) { ProcessProperties(DetailBuilder); }

void FGorgeousGlobalDetailCustomization::ProcessProperties(IDetailLayoutBuilder& DetailBuilder)
{
    TArray<TWeakObjectPtr<UObject>> Objects;
    DetailBuilder.GetObjectsBeingCustomized(Objects);

    if (Objects.Num() == 0 || !Objects[0].IsValid()) return;

    // This gets the ACTUAL instanced class (the Adapter), not the base class!
    UClass* ObjectClass = Objects[0]->GetClass();
    if (!ObjectClass) return;

    static const FName MetaDataTag = TEXT("GorgeousDetailExtension");

    for (TFieldIterator<FProperty> It(ObjectClass); It; ++It)
    {
       FProperty* Property = *It;
       if (Property->HasMetaData(MetaDataTag))
       {
          FName ExtensionName = *Property->GetMetaData(MetaDataTag);

          if (TStrongObjectPtr<UGorgeousDetailExtension>* ExtensionPtr = ExtensionRegistry.Find(ExtensionName))
          {
             // THE CRITICAL FIX: By passing ObjectClass as the second parameter, 
             // Unreal can successfully find the property inside the polymorphic array!
             TSharedRef<IPropertyHandle> PropertyHandle = DetailBuilder.GetProperty(Property->GetFName(), ObjectClass);
             
             if (PropertyHandle->IsValidHandle())
             {
                FName CategoryName = Property->GetMetaData(TEXT("Category")).IsEmpty() ? TEXT("Default") : *Property->GetMetaData(TEXT("Category"));
                IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(CategoryName);

                DetailBuilder.HideProperty(PropertyHandle);
                FDetailWidgetRow& CustomRow = Category.AddCustomRow(Property->GetDisplayNameText());
                ExtensionPtr->Get()->CustomizeHeader(PropertyHandle, CustomRow);
             }
          }
       }
    }
}