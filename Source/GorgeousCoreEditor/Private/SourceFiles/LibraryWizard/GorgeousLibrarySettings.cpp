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
#include "LibraryWizard/GorgeousLibrarySettings.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Helpers/GorgeousPersistentData.h"
#include "Helpers/Macros/GorgeousConnectionHelperMacros.h"
#include "LibraryWizard/GorgeousRecompilationHelper.h"
#include "LibraryWizard/GorgeousNetworkConsent.h"
#include "LibraryWizard/GorgeousUpdateManager.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Editor.h"
#include "UObject/UnrealType.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousLibrarySettings Implementation
//=============================================================================

UGorgeousLibrarySettings::UGorgeousLibrarySettings()
{
	ApiOrigin = TEXT(GORGEOUS_API_DEFAULT_HOST);
	ValidationInterval = 10;
	ValidationCount = 0;
}

#if WITH_EDITOR
void UGorgeousLibrarySettings::PostInitProperties()
{
	Super::PostInitProperties();
	const FGorgeousPersistentData& Data = FGorgeousPersistentData::Get();
	ValidationInterval = Data.GetSystemValidationInterval();
	ValidationCount = Data.GetSystemValidationCount();
	bForceDevMode = Data.GetForceDevMode();
}

void UGorgeousLibrarySettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	const FName MemberName = PropertyChangedEvent.GetMemberPropertyName();
	if (MemberName == GET_MEMBER_NAME_CHECKED(UGorgeousLibrarySettings, ValidationInterval))
	{
		FGorgeousPersistentData::Get().SetSystemValidationInterval(ValidationInterval);
	}
	else if (MemberName == GET_MEMBER_NAME_CHECKED(UGorgeousLibrarySettings, bForceDevMode))
	{
		FGorgeousPersistentData::Get().SetForceDevMode(bForceDevMode);
	}
}
#endif

void UGorgeousLibrarySettings::ConfigureOnlineServices()
{
	GorgeousNetworkConsent::ShowPreferences();
}

void UGorgeousLibrarySettings::TriggerRecompilation()
{
	UGorgeousRecompilationHelper::TriggerGorgeousRecompilation();
}

void UGorgeousLibrarySettings::CheckForUpdates()
{
	if (GEditor)
	{
		if (UGorgeousUpdateManager* UpdateManager = GEditor->GetEditorSubsystem<UGorgeousUpdateManager>())
		{
			UpdateManager->CheckForUpdates();
		}
	}
}