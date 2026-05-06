// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/StatsFoundation/GorgeousStatSettings.h"

UGorgeousStatSettings::UGorgeousStatSettings()
{
	CategoryName = TEXT("Gorgeous Things");
	SectionName = TEXT("Gorgeous Stats");
	
	ChangeSignalRoot = FGameplayTag::RequestGameplayTag(TEXT("Signal.Stat.Changed"));
	RestoredSignal = FGameplayTag::RequestGameplayTag(TEXT("Signal.Stat.Restored"));
}

const FGorgeousStat_S* UGorgeousStatSettings::FindStatMetadata(const FGameplayTag& Tag) const
{
	return StatRegistry.Find(Tag);
}
