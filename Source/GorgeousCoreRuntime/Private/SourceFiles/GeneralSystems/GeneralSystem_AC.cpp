#include "GeneralSystems/GeneralSystem_AC.h"
#include "GeneralSystems/GeneralSystemConfiguration_PDA.h"
#include "GeneralSystems/GorgeousSystemProjectSettings.h"
#include "GeneralSystems/VisualDataGathering/GorgeousVDGConsent.h"

void UGeneralSystem_AC::OnRegister()
{
    Super::OnRegister();
    FGorgeousVDGConsent::RegisterBundledHandlers(this);
}

FInstancedStruct UGeneralSystem_AC::GetSystemProjectSettings() const
{
    const UGeneralSystemConfiguration_PDA* Configuration = ProjectSettingsConfiguration.LoadSynchronous();
    return Configuration ? UGorgeousSystemProjectSettings::Resolve(*Configuration) : FInstancedStruct();
}
