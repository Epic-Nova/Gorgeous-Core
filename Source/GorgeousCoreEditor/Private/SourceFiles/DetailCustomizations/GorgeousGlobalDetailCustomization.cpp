#include "DetailCustomizations/GorgeousGlobalDetailCustomization.h"
#include "DetailExtensions/GorgeousDetailExtension.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "IDetailCustomNodeBuilder.h"
#include "PropertyHandle.h"
#include "UObject/UObjectIterator.h"
#include "PropertyEditorModule.h"
#include "IPropertyTypeCustomization.h"
#include "DetailExtensions/GorgeousDetailExtensionSettings.h"
#include "UObject/UnrealType.h"

namespace
{
TArray<FName> RegisteredExtensionPropertyTypes;
TSharedPtr<IPropertyTypeIdentifier> ExtensionPropertyIdentifier;

UGorgeousDetailExtension* FindEnabledExtension(const IPropertyHandle& Handle)
{
    const FProperty* Property = Handle.GetProperty();
    if (!Property || !Property->HasMetaData(TEXT("GorgeousDetailExtension"))) return nullptr;
    const FName Name(*Property->GetMetaData(TEXT("GorgeousDetailExtension")));
    const UGorgeousDetailExtensionSettings* Settings = GetDefault<UGorgeousDetailExtensionSettings>();
    if (!Settings->bEnableDetailExtensions || Settings->DisabledExtensions.Contains(Name)) return nullptr;
    const TStrongObjectPtr<UGorgeousDetailExtension>* Found = FGorgeousGlobalDetailCustomization::ExtensionRegistry.Find(Name);
    return Found ? Found->Get() : nullptr;
}

class FGorgeousExtensionPropertyIdentifier final : public IPropertyTypeIdentifier
{
    virtual bool IsPropertyTypeCustomized(const IPropertyHandle& Handle) const override
    {
        return FindEnabledExtension(Handle) != nullptr;
    }
};

class FGorgeousExtensionPropertyCustomization final : public IPropertyTypeCustomization
{
public:
    static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<FGorgeousExtensionPropertyCustomization>(); }
    virtual void CustomizeHeader(TSharedRef<IPropertyHandle> Handle, FDetailWidgetRow& Row, IPropertyTypeCustomizationUtils&) override
    {
        Extension.Reset(FindEnabledExtension(*Handle));
        if (Extension.IsValid()) Extension->CustomizeHeader(Handle, Row);
    }
    virtual void CustomizeChildren(TSharedRef<IPropertyHandle> Handle, IDetailChildrenBuilder& Builder, IPropertyTypeCustomizationUtils&) override
    {
        if (Extension.IsValid()) Extension->CustomizeChildren(Handle, Builder);
    }
private:
    TStrongObjectPtr<UGorgeousDetailExtension> Extension;
};

class FGorgeousExtensionNodeBuilder final : public IDetailCustomNodeBuilder
{
public:
    FGorgeousExtensionNodeBuilder(UGorgeousDetailExtension* InExtension, TSharedRef<IPropertyHandle> InHandle)
        : Extension(InExtension), Handle(InHandle) {}
    virtual void GenerateHeaderRowContent(FDetailWidgetRow& Row) override { Extension->CustomizeHeader(Handle, Row); }
    virtual void GenerateChildContent(IDetailChildrenBuilder& Builder) override { Extension->CustomizeChildren(Handle, Builder); }
    virtual FName GetName() const override { return Handle->GetProperty() ? Handle->GetProperty()->GetFName() : NAME_None; }
    virtual TSharedPtr<IPropertyHandle> GetPropertyHandle() const override { return Handle; }
private:
    TStrongObjectPtr<UGorgeousDetailExtension> Extension;
    TSharedRef<IPropertyHandle> Handle;
};
}

TMap<FName, TStrongObjectPtr<UGorgeousDetailExtension>> FGorgeousGlobalDetailCustomization::ExtensionRegistry;

TSharedRef<IDetailCustomization> FGorgeousGlobalDetailCustomization::MakeInstance() { return MakeShareable(new FGorgeousGlobalDetailCustomization); }

void FGorgeousGlobalDetailCustomization::RegisterExtension(UGorgeousDetailExtension* Extension)
{
    if (!Extension) return;
    ExtensionRegistry.Add(Extension->GetExtensionName(), TStrongObjectPtr<UGorgeousDetailExtension>(Extension));
    RegisterPropertyCustomizations();
}

void FGorgeousGlobalDetailCustomization::RegisterPropertyCustomizations()
{
    FPropertyEditorModule* Module = FModuleManager::GetModulePtr<FPropertyEditorModule>(TEXT("PropertyEditor"));
    if (!Module) return;
    if (!ExtensionPropertyIdentifier.IsValid()) ExtensionPropertyIdentifier = MakeShared<FGorgeousExtensionPropertyIdentifier>();
    TSet<FName> Types = { FName(TEXT("ArrayProperty")), FName(TEXT("MapProperty")), FName(TEXT("SetProperty")) };
    for (TObjectIterator<UStruct> Struct; Struct; ++Struct)
    {
        for (TFieldIterator<FProperty> Property(*Struct, EFieldIteratorFlags::ExcludeSuper); Property; ++Property)
        {
            if (!Property->HasMetaData(TEXT("GorgeousDetailExtension"))) continue;
            if (const FStructProperty* StructProperty = CastField<FStructProperty>(*Property)) Types.Add(StructProperty->Struct->GetFName());
            else if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(*Property)) Types.Add(EnumProperty->GetEnum()->GetFName());
            else if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(*Property)) Types.Add(ObjectProperty->PropertyClass->GetFName());
            else Types.Add(Property->GetClass()->GetFName());
        }
    }
    for (FName Type : Types)
    {
        if (RegisteredExtensionPropertyTypes.Contains(Type)) continue;
        Module->RegisterCustomPropertyTypeLayout(Type,
            FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGorgeousExtensionPropertyCustomization::MakeInstance), ExtensionPropertyIdentifier);
        RegisteredExtensionPropertyTypes.Add(Type);
    }
    Module->NotifyCustomizationModuleChanged();
}

void FGorgeousGlobalDetailCustomization::UnregisterPropertyCustomizations()
{
    if (FPropertyEditorModule* Module = FModuleManager::GetModulePtr<FPropertyEditorModule>(TEXT("PropertyEditor")))
    {
        for (FName Type : RegisteredExtensionPropertyTypes) Module->UnregisterCustomPropertyTypeLayout(Type, ExtensionPropertyIdentifier);
        Module->NotifyCustomizationModuleChanged();
    }
    RegisteredExtensionPropertyTypes.Empty();
    ExtensionPropertyIdentifier.Reset();
}

void FGorgeousGlobalDetailCustomization::UnregisterExtension(FName ExtensionName) { ExtensionRegistry.Remove(ExtensionName); }
void FGorgeousGlobalDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) { ProcessProperties(DetailBuilder); }

void FGorgeousGlobalDetailCustomization::ProcessProperties(IDetailLayoutBuilder& DetailBuilder)
{
    TArray<TWeakObjectPtr<UObject>> Objects;
    DetailBuilder.GetObjectsBeingCustomized(Objects);

    if (Objects.Num() == 0 || !Objects[0].IsValid()) return;

    // This gets the ACTUAL instanced class (the Adapter), not the base class!
    UClass* ObjectClass = DetailBuilder.GetBaseClass();
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
             TSharedRef<IPropertyHandle> PropertyHandle = DetailBuilder.GetProperty(Property->GetFName(), Property->GetOwnerClass());
             
             if (PropertyHandle->IsValidHandle())
             {
                FName CategoryName = Property->GetMetaData(TEXT("Category")).IsEmpty() ? TEXT("Default") : *Property->GetMetaData(TEXT("Category"));
                IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(CategoryName);

                DetailBuilder.HideProperty(PropertyHandle);
                Category.AddCustomBuilder(MakeShared<FGorgeousExtensionNodeBuilder>(ExtensionPtr->Get(), PropertyHandle));
             }
          }
       }
    }
}
