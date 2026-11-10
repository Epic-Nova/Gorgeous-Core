#include "PropertyTypeCustomizations/GorgeousObjectVariablePropertyTypeCustomization.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/GorgeousObjectVariableTrunk.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "IPropertyUtilities.h"
#include "PropertyHandle.h"
#include "ScopedTransaction.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<IPropertyTypeCustomization> FGorgeousObjectVariablePropertyTypeCustomization::MakeInstance()
{ return MakeShared<FGorgeousObjectVariablePropertyTypeCustomization>(); }
void FGorgeousObjectVariablePropertyTypeCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> Handle,FDetailWidgetRow& Row,IPropertyTypeCustomizationUtils& Utils)
{
    PayloadHandle=Handle; PropertyUtilities=Utils.GetPropertyUtilities();
    Row.NameContent()[Handle->CreatePropertyNameWidget()];
    Row.ValueContent()[SNew(STextBlock).Text(NSLOCTEXT("GorgeousOVPayload","Summary","Object Variable Payload"))];
}
void FGorgeousObjectVariablePropertyTypeCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> Handle,IDetailChildrenBuilder& Builder,IPropertyTypeCustomizationUtils& Utils)
{
    PayloadHandle=Handle; PropertyUtilities=Utils.GetPropertyUtilities();
    auto ClassHandle=Handle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGorgeousObjectVariableSerializedPayload,VariableClass));
    auto ShowAll=Handle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGorgeousObjectVariableSerializedPayload,bShowInnerProperties));
    if (ClassHandle) { Builder.AddProperty(ClassHandle.ToSharedRef()); ClassHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this,&FGorgeousObjectVariablePropertyTypeCustomization::ClassChanged)); }
    if (ShowAll) { Builder.AddProperty(ShowAll.ToSharedRef()); ShowAll->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this,&FGorgeousObjectVariablePropertyTypeCustomization::Refresh)); }
    TArray<void*> Data; Handle->AccessRawData(Data);
    TArray<UObject*> Objects; UClass* CommonClass=nullptr; bool bAllFields=false;
    Previews.Reset();
    for (void* Raw:Data)
    {
        const auto* Payload=static_cast<FGorgeousObjectVariableSerializedPayload*>(Raw);
        if (!Payload || !Payload->VariableClass || Payload->VariableClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated)) continue;
        if (CommonClass && CommonClass!=Payload->VariableClass.Get()) { Objects.Reset(); break; }
        CommonClass=Payload->VariableClass;
        auto* Preview=NewObject<UGorgeousObjectVariable>(GetTransientPackage(),CommonClass,NAME_None,RF_Transactional | RF_Transient);
        if (Payload->IsValid()) Preview->DeserializeFromPayload(*Payload);
        Preview->UniqueIdentifier=Payload->VariableIdentifier.IsValid() ? Payload->VariableIdentifier : FGuid::NewGuid();
        Previews.Emplace(Preview); Objects.Add(Preview); bAllFields|=Payload->bShowInnerProperties;
    }
    if (Objects.IsEmpty()) return;
    bAllFields|=FindFProperty<FProperty>(CommonClass,TEXT("Value"))==nullptr;
    for (TFieldIterator<FProperty> It(CommonClass);It;++It)
    {
        FProperty* Property=*It;
        if (!Property->HasAnyPropertyFlags(CPF_Edit) || Property->HasAnyPropertyFlags(CPF_EditConst | CPF_Transient | CPF_DisableEditOnInstance)) continue;
        if (!bAllFields && Property->GetFName()!=TEXT("Value")) continue;
        if (IDetailPropertyRow* Row=Builder.AddExternalObjectProperty(Objects,Property->GetFName()))
            if (auto ValueHandle=Row->GetPropertyHandle()) ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this,&FGorgeousObjectVariablePropertyTypeCustomization::SavePreview));
    }
}
void FGorgeousObjectVariablePropertyTypeCustomization::SavePreview()
{
    if (bSaving) return;
    auto Handle=PayloadHandle.Pin(); if (!Handle) return;
    TGuardValue<bool> Guard(bSaving,true);
    TArray<void*> Data; Handle->AccessRawData(Data); if (Data.Num()!=Previews.Num()) return;
    FScopedTransaction Transaction(NSLOCTEXT("GorgeousOVPayload","Edit","Edit Object Variable Payload"));
    TArray<UObject*> Owners; Handle->GetOuterObjects(Owners); for (auto* Owner:Owners) if (Owner) Owner->Modify();
    Handle->NotifyPreChange();
    for (int32 Index=0;Index<Data.Num();++Index)
    {
        auto* Payload=static_cast<FGorgeousObjectVariableSerializedPayload*>(Data[Index]);
        if (Payload && Previews[Index].IsValid()) Previews[Index]->SerializeToPayload(*Payload);
    }
    Handle->NotifyPostChange(EPropertyChangeType::ValueSet); Handle->NotifyFinishedChangingProperties();
}
void FGorgeousObjectVariablePropertyTypeCustomization::ClassChanged()
{
    if (auto Handle=PayloadHandle.Pin())
    {
        TArray<void*> Data; Handle->AccessRawData(Data);
        for (void* Raw:Data) if (auto* Payload=static_cast<FGorgeousObjectVariableSerializedPayload*>(Raw)) { Payload->ResetSerializedState(); Payload->ApplyMetadataFromClass(Payload->VariableClass); }
    }
    Refresh();
}
void FGorgeousObjectVariablePropertyTypeCustomization::Refresh()
{ if (auto Utilities=PropertyUtilities.Pin()) Utilities->RequestRefresh(); }
