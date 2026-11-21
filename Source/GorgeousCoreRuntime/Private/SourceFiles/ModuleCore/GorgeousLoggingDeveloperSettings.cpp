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
#include "GorgeousLoggingDeveloperSettings.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/StringOutputDevice.h"
#include "UObject/UnrealType.h"

UGorgeousLoggingDeveloperSettings::UGorgeousLoggingDeveloperSettings()
{
	bEnableGorgeousMessageLog = true;
	MessageLogListingName = TEXT("Gorgeous Things");
	MinMessageLogVerbosity = Logging_Information;
	bMirrorToOutputLog = true;
	bShowOnScreen = true;
	bShowInGame = false;
}

void UGorgeousLoggingDeveloperSettings::PostInitProperties()
{
	Super::PostInitProperties();
	if (!HasAnyFlags(RF_ClassDefaultObject) || !GConfig || !bUseLegacyInsightChartRegistry) return;

	FString TextValue;
	const TCHAR* PropertyName = TEXT("RegisteredUMGCharts");
	if (GConfig->GetString(*GetClass()->GetPathName(), PropertyName, TextValue, GGameIni)
		|| !GConfig->GetString(TEXT("/Script/GorgeousCoreRuntime.GorgeousInsightChartRegistry"), PropertyName, TextValue, GGameIni))
	{
		return;
	}

	const auto* Property = FindFProperty<FMapProperty>(GetClass(),
		GET_MEMBER_NAME_CHECKED(UGorgeousLoggingDeveloperSettings, RegisteredUMGCharts));
	decltype(RegisteredUMGCharts) LegacyCharts;
	FStringOutputDevice Errors;
	const TCHAR* End = Property->ImportText_Direct(*TextValue, &LegacyCharts, this, PPF_None, &Errors);
	if (End && *End == 0 && Errors.IsEmpty())
	{
		RegisteredUMGCharts = MoveTemp(LegacyCharts);
	}
}

#if WITH_EDITOR
void UGorgeousLoggingDeveloperSettings::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	if (Event.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UGorgeousLoggingDeveloperSettings, RegisteredUMGCharts))
	{
		bUseLegacyInsightChartRegistry = false;
	}
	Super::PostEditChangeProperty(Event);
}
#endif
