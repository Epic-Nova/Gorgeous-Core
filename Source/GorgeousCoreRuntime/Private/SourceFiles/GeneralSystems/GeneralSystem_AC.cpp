#include "GeneralSystems/GeneralSystem_AC.h"
#include "GeneralSystems/VisualDataGathering/GorgeousVDGConsent.h"

void UGeneralSystem_AC::OnRegister()
{
    Super::OnRegister();
    FGorgeousVDGConsent::RegisterBundledHandlers(this);
}
