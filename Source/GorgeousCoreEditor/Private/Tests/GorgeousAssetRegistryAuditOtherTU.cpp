#if WITH_DEV_AUTOMATION_TESTS
#include "AssetRegistration/GorgeousAssetRegistrationHelpers.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Modules/ModuleManager.h"
void RegisterGorgeousAuditActionFromOtherTU(FName Owner, const TSharedRef<IAssetTypeActions>& Action)
{
    FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().RegisterAssetTypeActions(Action);
    GorgeousAssetRegistration::TrackAssetTypeAction(Owner, Action);
}
const void* GetGorgeousAuditCategoryAddressFromOtherTU()
{
    return &GorgeousAssetRegistration::GGorgeousThingsCategory;
}
#endif
