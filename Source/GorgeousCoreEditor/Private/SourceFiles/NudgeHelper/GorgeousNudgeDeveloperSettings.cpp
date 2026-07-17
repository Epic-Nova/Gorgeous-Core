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
#include "NudgeHelper/GorgeousNudgeDeveloperSettings.h"

//=============================================================================
// UGorgeousNudgeDeveloperSettings Implementation
//=============================================================================

const UGorgeousNudgeDeveloperSettings* UGorgeousNudgeDeveloperSettings::Get()
{
	return GetDefault<UGorgeousNudgeDeveloperSettings>();
}

FText UGorgeousNudgeDeveloperSettings::GetSectionText() const
{
	return NSLOCTEXT("GorgeousNudgeDeveloperSettings", "SectionName", "Nudges");
}
