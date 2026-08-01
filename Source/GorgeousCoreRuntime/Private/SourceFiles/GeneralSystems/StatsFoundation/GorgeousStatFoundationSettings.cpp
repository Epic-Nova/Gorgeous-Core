// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundationSettings.h"
#include "GeneralSystems/StatsFoundation/GorgeousStatFoundationTags.h"

UGorgeousStatFoundationSettings::UGorgeousStatFoundationSettings()
{
	CategoryName = TEXT("Gorgeous Things");
	SectionName = TEXT("Gorgeous Stats");
	
	ChangeSignalRoot = TAG_Gorgeous_Signal_Stat_Changed;
	RestoredSignal = TAG_Gorgeous_Signal_Stat_Restored;
}

const FGorgeousStat_S* UGorgeousStatFoundationSettings::FindStatMetadata(const FGameplayTag& Tag) const
{
	return StatRegistry.Find(Tag);
}
