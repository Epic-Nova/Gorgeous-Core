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
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "UObject/StrongObjectPtr.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"
//<-------------------------------------------------------------------------->

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