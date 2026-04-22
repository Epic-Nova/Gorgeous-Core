// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - System developer settings implementation    |
<==========================================================================*/
#include "ModuleCore/GorgeousSystemDeveloperSettings.h"
#include "GeneralSystems/GeneralSystemConfiguration_PDA.h"

UGorgeousSystemDeveloperSettings::UGorgeousSystemDeveloperSettings()
{
}

UGorgeousSystemDeveloperSettings* UGorgeousSystemDeveloperSettings::Get()
{
    return GetMutableDefault<UGorgeousSystemDeveloperSettings>();
}

#if WITH_EDITOR
FText UGorgeousSystemDeveloperSettings::GetSectionText() const
{
    return NSLOCTEXT("GorgeousSystemDeveloperSettings", "SectionName", "Core ↪ System Overrides");
}
#endif
