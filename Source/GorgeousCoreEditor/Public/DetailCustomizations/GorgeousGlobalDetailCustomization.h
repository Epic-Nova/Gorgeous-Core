#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"
#include "UObject/StrongObjectPtr.h"

class UGorgeousDetailExtension;

class GORGEOUSCOREEDITOR_API FGorgeousGlobalDetailCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static void RegisterExtension(UGorgeousDetailExtension* Extension);
	static void UnregisterExtension(FName ExtensionName);

	static TMap<FName, TStrongObjectPtr<UGorgeousDetailExtension>> ExtensionRegistry;
private:
	void ProcessProperties(IDetailLayoutBuilder& DetailBuilder);
    
	friend class FGorgeousCoreEditorModule;
};