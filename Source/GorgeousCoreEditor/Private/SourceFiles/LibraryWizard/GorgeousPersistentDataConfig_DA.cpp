#include "LibraryWizard/GorgeousPersistentDataConfig_DA.h"
#include "Helpers/GorgeousPluginHelper.h"

#if WITH_EDITOR
void UGorgeousPersistentDataConfig_DA::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (UGorgeousPluginHelper* Helper = UGorgeousPluginHelper::GetSingleton())
	{
		// Map our edited properties back to the actual JSON representation
		Helper->SetSystemValidationInterval(ValidationInterval);
		Helper->SetSystemValidationCount(ValidationCount);
		Helper->SetHasRunInitialValidation(bHasRunInitialValidation);
		Helper->SetPluginUpdateCache(PluginUpdateCache);
		Helper->SetForceDevMode(bForceDevMode);

		// Instantly write the modifications to GorgeousPersistentData.json
		Helper->SavePersistentData();
	}
}
#endif
