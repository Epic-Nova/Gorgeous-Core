// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSettings.h"

UGorgeousUIFoundationSettings::UGorgeousUIFoundationSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (DefaultLayoutClass.IsNull())
	{
		DefaultLayoutClass = TSoftClassPtr<UGorgeousPrimaryGameLayout>(
			FSoftObjectPath(TEXT("/GorgeousCore/Systems/CommonUIFoundation/UserInterfaces/BP_DefaultPrimaryGameLayout_CUW.BP_DefaultPrimaryGameLayout_CUW")));
		
		DefaultTheme = TSoftObjectPtr<UGorgeousUITheme_DA>(
			FSoftObjectPath(TEXT("/GorgeousCore/Systems/CommonUIFoundation/UserInterfaces/Themes/DT_Theme_Generic.DT_Theme_Generic")));
		
		DefaultInputBindings = TSoftObjectPtr<UGorgeousInputBinding_DA>(
			FSoftObjectPath(TEXT("/GorgeousCore/Systems/CommonUIFoundation/Input/DA_InputBinding_Generic.DA_InputBinding_Generic")));
	}
}
