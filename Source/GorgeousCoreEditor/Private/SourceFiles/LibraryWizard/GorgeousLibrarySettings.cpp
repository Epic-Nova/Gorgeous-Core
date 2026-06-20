#include "LibraryWizard/GorgeousLibrarySettings.h"
#include "Helpers/GorgeousPluginHelper.h"
#include "Misc/App.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "LibraryWizard/GorgeousUpdateManager.h"
#include "LibraryWizard/GorgeousRecompilationHelper.h"
#include "Editor.h"

UGorgeousLibrarySettings::UGorgeousLibrarySettings()
{
	ValidationInterval = 5;
	ValidationCount = 0;
}

#if WITH_EDITOR
void UGorgeousLibrarySettings::PostInitProperties()
{
	Super::PostInitProperties();
	if (UGorgeousPluginHelper* Helper = UGorgeousPluginHelper::GetSingleton())
	{
		ValidationInterval = Helper->GetSystemValidationInterval();
		ValidationCount = Helper->GetSystemValidationCount();
	}
}

void UGorgeousLibrarySettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UGorgeousLibrarySettings, ValidationInterval))
	{
		if (UGorgeousPluginHelper* Helper = UGorgeousPluginHelper::GetSingleton())
		{
			Helper->SetSystemValidationInterval(ValidationInterval);
		}
	}
}
#endif

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
