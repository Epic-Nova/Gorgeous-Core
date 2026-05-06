// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"

class UGorgeousDetailExtension;

/**
 * Global customization bridge that scans for GorgeousDetailExtension metadata.
 * Registered for UObject to ensure coverage across all classes.
 */
class FGorgeousGlobalDetailCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	// IDetailCustomization Interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	// End IDetailCustomization Interface

	/** Registers an extension instance. */
	static void RegisterExtension(UGorgeousDetailExtension* Extension);

	/** Unregisters an extension instance. */
	static void UnregisterExtension(FName ExtensionName);

private:
	/** Registry of available extensions. */
	static TMap<FName, TStrongObjectPtr<UGorgeousDetailExtension>> ExtensionRegistry;

	/** Scans the class for properties with GorgeousDetailExtension metadata. */
	void ProcessProperties(IDetailLayoutBuilder& DetailBuilder);

	friend class FGorgeousCoreEditorModule;
};
