#pragma once
#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "UObject/StrongObjectPtr.h"
class UGorgeousObjectVariable;
class IPropertyUtilities;

/** Edits serialized OV payloads through transient concrete instances, committing changes back transactionally. */
class FGorgeousObjectVariablePropertyTypeCustomization : public IPropertyTypeCustomization
{
public:
    static TSharedRef<IPropertyTypeCustomization> MakeInstance();
    virtual void CustomizeHeader(TSharedRef<IPropertyHandle> Handle,FDetailWidgetRow& Row,IPropertyTypeCustomizationUtils& Utils) override;
    virtual void CustomizeChildren(TSharedRef<IPropertyHandle> Handle,IDetailChildrenBuilder& Builder,IPropertyTypeCustomizationUtils& Utils) override;
private:
    void SavePreview();
    void ClassChanged();
    void Refresh();
    TWeakPtr<IPropertyHandle> PayloadHandle;
    TWeakPtr<IPropertyUtilities> PropertyUtilities;
    TArray<TStrongObjectPtr<UGorgeousObjectVariable>> Previews;
    bool bSaving=false;
};
