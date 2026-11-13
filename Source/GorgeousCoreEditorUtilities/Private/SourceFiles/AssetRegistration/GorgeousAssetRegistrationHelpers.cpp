#include "AssetRegistration/GorgeousAssetRegistrationHelpers.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Modules/ModuleManager.h"

namespace GorgeousAssetRegistration
{
    EAssetTypeCategories::Type GGorgeousThingsCategory = EAssetTypeCategories::Misc;
    // One exported registry shared across translation units, retaining module ownership.
    static TMap<FName, TArray<TSharedPtr<IAssetTypeActions>>> RegisteredActions;

    static void UnregisterIfCurrent(IAssetTools& Tools, const TSharedRef<IAssetTypeActions>& Action)
    {
        // UE replaces actions by supported class and its Unregister also removes
        // by class. A stale owner must not unregister a later module's replacement.
        if (Tools.GetAssetTypeActionsForClass(Action->GetSupportedClass()).Pin() == Action)
            Tools.UnregisterAssetTypeActions(Action);
    }

    void TrackAssetTypeAction(FName OwningModule, const TSharedRef<IAssetTypeActions>& Action)
    {
        RegisteredActions.FindOrAdd(OwningModule).AddUnique(Action);
    }

    void UnregisterAssetTypeAction(const TSharedRef<IAssetTypeActions>& Action)
    {
        if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
            UnregisterIfCurrent(FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get(), Action);
        for (auto It = RegisteredActions.CreateIterator(); It; ++It)
        {
            It.Value().Remove(Action);
            if (It.Value().IsEmpty()) It.RemoveCurrent();
        }
    }

    void UnregisterAssetTypeActions(FName OwningModule)
    {
        TArray<TSharedPtr<IAssetTypeActions>> Actions;
        if (!RegisteredActions.RemoveAndCopyValue(OwningModule, Actions)) return;
        if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
        {
            IAssetTools& Tools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
            for (const auto& Action : Actions)
                if (Action.IsValid()) UnregisterIfCurrent(Tools, Action.ToSharedRef());
        }
    }
}
