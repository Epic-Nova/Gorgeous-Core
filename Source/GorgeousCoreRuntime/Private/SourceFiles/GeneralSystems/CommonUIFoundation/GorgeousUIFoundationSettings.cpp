// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSettings.h"

UGorgeousUIFoundationSettings::UGorgeousUIFoundationSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (DefaultLayoutClass.IsNull())
	{
		DefaultLayoutClass = TSoftClassPtr<UGorgeousPrimaryGameLayout>(
			FSoftObjectPath(TEXT("/Script/GorgeousCoreRuntime.GorgeousPrimaryGameLayout")));
	}
}
