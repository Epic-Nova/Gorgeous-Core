#include "GeneralSystems/VisualDataGathering/GorgeousVDGStoreRouting.h"
#include "GeneralSystems/GeneralSystemConfiguration_PDA.h"
#include "GeneralSystems/GeneralSystem_AC.h"
#include "QualityOfLife/GorgeousGameInstance.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/UnrealType.h"

bool UGorgeousVDGStoreRouting::ShouldUseLocalVDGStore(const UObject* WorldContextObject)
{
    UWorld* World=GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject,EGetWorldErrorMode::ReturnNull) : nullptr;
    auto* Instance=World ? Cast<UGorgeousGameInstance>(World->GetGameInstance()) : nullptr;
    const auto* Entry=Instance ? Instance->AdditionalGorgeousData.Find(TEXT("GeneralBlueprintSystemsRuntime")) : nullptr;
    UGorgeousObjectVariable* Runtime=Entry ? Entry->Handle.GetCachedValue() : nullptr;
    if (!Runtime && Entry) Runtime=Entry->DefaultValue;
    auto* Active=Runtime ? FindFProperty<FArrayProperty>(Runtime->GetClass(),TEXT("ActiveSystems")) : nullptr;
    auto* Object=Active ? CastField<FObjectProperty>(Active->Inner) : nullptr;
    if (!Active || !Object) return true;

    bool bTeam=false,bPlaylist=false;
    FScriptArrayHelper Items(Active,Active->ContainerPtrToValuePtr<void>(Runtime));
    const auto Configurations=UGeneralSystemConfiguration_PDA::GetAllSystemConfigurations();
    for (int32 Index=0;Index<Items.Num();++Index)
    {
        auto* Component=Cast<UActorComponent>(Object->GetObjectPropertyValue(Items.GetRawPtr(Index)));
        if (!IsValid(Component) || !Component->IsRegistered() || Component->GetWorld()!=World) continue;
        bTeam |= Component->ComponentTags.Contains(TEXT("Team_System"));
        bPlaylist |= Component->ComponentTags.Contains(TEXT("Playlist_System"));
        // Keep the existing configuration-name/tag aliases, but only count a
        // configuration when its actual component is enrolled in this world.
        for (const auto* Configuration:Configurations)
            if (IsValid(Configuration) && Configuration->AuthoritativeComponent && Component->IsA(Configuration->AuthoritativeComponent))
            {
                bTeam |= Configuration->GetPDAAssetName()==TEXT("PDA_Team");
                bPlaylist |= Configuration->GetPDAAssetName()==TEXT("PDA_Playlist");
            }
    }
    return !(bTeam && bPlaylist);
}
