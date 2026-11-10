#include "GeneralSystems/VisualDataGathering/GorgeousVDGConsent.h"
#include "GeneralSystems/CommonUIFoundation/Actions/AsyncAction_PushGorgeousMessage.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIMessageConfig_DA.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "QualityOfLife/GorgeousPlayerState.h"
#include "QualityOfLife/GorgeousPlayerConnectionInfo_I.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRelayComponent.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "Blueprint/WidgetTree.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "UObject/UnrealType.h"

namespace
{
    bool HasBundledAncestor(const UObject* Object,const TCHAR* Path)
    {
        for (const UClass* Class=Object ? Object->GetClass() : nullptr;Class;Class=Class->GetSuperClass())
            if (Class->GetPathName()==Path) return true;
        return false;
    }
}

void UGorgeousVDGConsentDialog::InitializeMessage_Implementation(const FGorgeousUIMessageRequest& Request)
{
    Super::InitializeMessage_Implementation(Request);
    if (!WidgetTree) WidgetTree=NewObject<UWidgetTree>(this);
    auto* Root=WidgetTree->ConstructWidget<UVerticalBox>();
    WidgetTree->RootWidget=Root;
    auto* Title=WidgetTree->ConstructWidget<UTextBlock>(); Title->SetText(Request.Title); Root->AddChild(Title);
    auto* Body=WidgetTree->ConstructWidget<UTextBlock>(); Body->SetText(Request.Message); Body->SetAutoWrapText(true); Root->AddChild(Body);
    auto* Allow=WidgetTree->ConstructWidget<UButton>();
    auto* AllowText=WidgetTree->ConstructWidget<UTextBlock>(); AllowText->SetText(NSLOCTEXT("GorgeousVDG","Allow","Allow for this session"));
    Allow->AddChild(AllowText); Root->AddChild(Allow); Allow->OnClicked.AddDynamic(this,&ThisClass::Accept);
    DeclineButton=WidgetTree->ConstructWidget<UButton>();
    auto* DeclineText=WidgetTree->ConstructWidget<UTextBlock>(); DeclineText->SetText(NSLOCTEXT("GorgeousVDG","Decline","Do not allow"));
    DeclineButton->AddChild(DeclineText); Root->AddChild(DeclineButton); DeclineButton->OnClicked.AddDynamic(this,&ThisClass::Decline);
}
void UGorgeousVDGConsentDialog::NativeOnActivated()
{
    Super::NativeOnActivated();
    if (DeclineButton) DeclineButton->SetUserFocus(GetOwningPlayer());
}
void UGorgeousVDGConsentDialog::Accept() { SubmitMessageResult(TEXT("Confirm")); }
void UGorgeousVDGConsentDialog::Decline() { SubmitMessageResult(TEXT("Cancel")); }

UAsyncAction_RequestVDGConsent* UAsyncAction_RequestVDGConsent::RequestVDGConsent(UObject* WorldContextObject)
{
    auto* Action=NewObject<UAsyncAction_RequestVDGConsent>(); Action->Context=WorldContextObject;
    if (WorldContextObject) Action->RegisterWithGameInstance(WorldContextObject);
    return Action;
}
void UAsyncAction_RequestVDGConsent::Activate()
{
    auto* PC=FGorgeousVDGConsent::ResolveController(Context);
    if (!PC || !PC->IsLocalController()) { Finish(false); return; }
    if (PC->HasVisualDataGatheringConsentDecision()) { Finish(PC->HasVisualDataGatheringConsent()); return; }
    DialogConfig=NewObject<UGorgeousUIMessageConfig_DA>(this);
    DialogConfig->MessageWidgetClass=UGorgeousVDGConsentDialog::StaticClass();
    DialogConfig->LayerTag=TAG_Gorgeous_UI_Layer_Message.GetTag();
    DialogConfig->Buttons.Add(TEXT("Confirm"),NSLOCTEXT("GorgeousVDG","Allow","Allow for this session"));
    DialogConfig->Buttons.Add(TEXT("Cancel"),NSLOCTEXT("GorgeousVDG","Decline","Do not allow"));
    if (!DialogConfig->LayerTag.IsValid()) { Finish(false); return; }
    DialogAction=UAsyncAction_PushGorgeousMessage::PushGorgeousMessage(PC,
        NSLOCTEXT("GorgeousVDG","Title","Share survey answers?"),
        NSLOCTEXT("GorgeousVDG","Explanation","Allow surveys to collect and send your answers to this game's server for the current session? You can decline or revoke this choice."),DialogConfig);
    if (!DialogAction) { Finish(false); return; }
    DialogAction->OnConfirmed.AddDynamic(this,&ThisClass::Accepted);
    DialogAction->OnCancelled.AddDynamic(this,&ThisClass::Declined);
    DialogAction->OnCustomResult.AddDynamic(this,&ThisClass::Declined);
    DialogAction->Activate();
}
void UAsyncAction_RequestVDGConsent::Accepted(FName) { Finish(true); }
void UAsyncAction_RequestVDGConsent::Declined(FName) { Finish(false); }
void UAsyncAction_RequestVDGConsent::Finish(bool bAccepted)
{
    if (bFinished) return; bFinished=true;
    auto* PC=FGorgeousVDGConsent::ResolveController(Context);
    if (!PC || !PC->IsLocalController()) bAccepted=false;
    if (PC) PC->SetVisualDataGatheringConsent(bAccepted);
    if (bAccepted) OnAccepted.Broadcast(); else OnDeclined.Broadcast();
    SetReadyToDestroy();
}

AGorgeousPlayerController* FGorgeousVDGConsent::ResolveController(const UObject* Context)
{
    if (const auto* Component=Cast<UActorComponent>(Context)) Context=Component->GetOwner();
    if (auto* PC=Cast<AGorgeousPlayerController>(const_cast<UObject*>(Context))) return PC;
    if (const auto* Pawn=Cast<APawn>(Context)) return Cast<AGorgeousPlayerController>(Pawn->GetController());
    if (const auto* Widget=Cast<UUserWidget>(Context)) return Cast<AGorgeousPlayerController>(Widget->GetOwningPlayer());
    return nullptr;
}
bool FGorgeousVDGConsent::IsCollectionRequest(FName Key)
{
    return Key==TEXT("RequestVDGPropertyWrite") || Key==TEXT("RequestVDGPropertySync") || Key==TEXT("FetchCurrentMembersForVDGProperty") ||
        Key==TEXT("WriteVDG Property") || Key==TEXT("SyncVDG Property") || Key==TEXT("NetGetMaxVDGPropertyMembers");
}
bool FGorgeousVDGConsent::CanSendCollectionRequest(const UObject* Context)
{
    const auto* PC=ResolveController(Context);
    return PC && PC->IsLocalController() && PC->HasVisualDataGatheringConsent();
}
bool FGorgeousVDGConsent::IsBundledHandler(const UObject* Object,FName Handler)
{
    if (Handler==TEXT("WriteVDG Property") || Handler==TEXT("SyncVDG Property"))
        return HasBundledAncestor(Object,TEXT("/GorgeousCore/Systems/VisualDataGeathering/Classes/ActorComponents/BP_VisualDataGeatheringManager_AC.BP_VisualDataGeatheringManager_AC_C"));
    return Handler==TEXT("NetGetMaxVDGPropertyMembers") && HasBundledAncestor(Object,TEXT("/GorgeousCore/Systems/Playlist/Classes/ActorComponents/BP_PlaylistHandling_AC.BP_PlaylistHandling_AC_C"));
}
void FGorgeousVDGConsent::RegisterBundledHandlers(UObject* Object)
{
    const auto* Component=Cast<UActorComponent>(Object);
    if (!Component || !Component->GetOwner() || !Component->GetOwner()->HasAuthority()) return;
    for (FName Name:{FName(TEXT("WriteVDG Property")),FName(TEXT("SyncVDG Property")),FName(TEXT("NetGetMaxVDGPropertyMembers"))})
        if (IsBundledHandler(Object,Name)) UGorgeousObjectVariable::RegisterAutoReplicationRPCHandlerOnObject(Object,Name);
}
bool FGorgeousVDGConsent::ValidateInvocation(UObject* Object,const FGorgeousQueuedRPC& Request,const UFunction* Function,const void* Parameters)
{
    if (!IsBundledHandler(Object,Request.Payload.HandlerName)) return true;
    if (Request.Type!=EGorgeousAutoReplicationRPCType::EReliableServer && Request.Type!=EGorgeousAutoReplicationRPCType::EUnreliableServer) return false;
    auto* Origin=UGorgeousAutoReplicationRPCRelayComponent::ResolveClientInitiatedRPCOrigin(Request.RequestGuid);
    // Locally authored authority operations have no remote origin. Remote relay
    // ingress always records its actual owning controller before dispatch.
    if (!Origin) return Object->GetWorld() && Object->GetWorld()->GetNetMode()!=NM_Client;
    auto* PC=Cast<AGorgeousPlayerController>(Origin);
    if (!PC || PC->GetWorld()!=Object->GetWorld() || !PC->HasVisualDataGatheringConsent()) return false;
    if (const auto* PlayerId=FindFProperty<FStrProperty>(Function,TEXT("PlayerStableID")))
    {
        const UObject* Identity=PC->PlayerState;
        if (!Identity || !Identity->GetClass()->ImplementsInterface(UGorgeousPlayerConnectionInfo_I::StaticClass())) return false;
        const FString ActualId=IGorgeousPlayerConnectionInfo_I::Execute_GetGorgeousStablePlayerId(const_cast<UObject*>(Identity));
        return !ActualId.IsEmpty() && Parameters && PlayerId->GetPropertyValue_InContainer(Parameters)==ActualId;
    }
    return Request.Payload.HandlerName==TEXT("NetGetMaxVDGPropertyMembers");
}
