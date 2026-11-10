// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "DetailExtensions/GorgeousDetailExtensionSettings.h"
#include "PropertyEditorModule.h"

void UGorgeousDetailExtensionSettings::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);
	SaveConfig();
	if (FPropertyEditorModule* Module = FModuleManager::GetModulePtr<FPropertyEditorModule>(TEXT("PropertyEditor")))
		Module->NotifyCustomizationModuleChanged();
}
