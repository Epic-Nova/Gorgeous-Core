#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousActivatableWidget.h"
#include "GorgeousVDGConsent.generated.h"

class AGorgeousPlayerController;
class UAsyncAction_PushGorgeousMessage;
class UGorgeousUIMessageConfig_DA;
class UButton;
struct FGorgeousQueuedRPC;

/** A concrete two-choice dialog for the bundled survey's session consent. */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousVDGConsentDialog : public UGorgeousActivatableWidget
{
    GENERATED_BODY()
public:
    virtual void InitializeMessage_Implementation(const FGorgeousUIMessageRequest& Request) override;
protected:
    virtual void NativeOnActivated() override;
    UFUNCTION() void Accept();
    UFUNCTION() void Decline();
    UPROPERTY(Transient) TObjectPtr<UButton> DeclineButton;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGorgeousVDGConsentDecision);

/** Collection remains stopped until the owning local player explicitly accepts. */
UCLASS()
class GORGEOUSCORERUNTIME_API UAsyncAction_RequestVDGConsent : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable,Category="Gorgeous Core|Visual Data Gathering",meta=(BlueprintInternalUseOnly="true",WorldContext="WorldContextObject"))
    static UAsyncAction_RequestVDGConsent* RequestVDGConsent(UObject* WorldContextObject);
    virtual void Activate() override;
    UPROPERTY(BlueprintAssignable) FGorgeousVDGConsentDecision OnAccepted;
    UPROPERTY(BlueprintAssignable) FGorgeousVDGConsentDecision OnDeclined;
private:
    UFUNCTION() void Accepted(FName Result);
    UFUNCTION() void Declined(FName Result);
    void Finish(bool bAccepted);
    UPROPERTY(Transient) TObjectPtr<UObject> Context;
    UPROPERTY(Transient) TObjectPtr<UGorgeousUIMessageConfig_DA> DialogConfig;
    UPROPERTY(Transient) TObjectPtr<UAsyncAction_PushGorgeousMessage> DialogAction;
    bool bFinished=false;
};

/** Narrow integration for the bundled VDG receiver functions; other systems are unchanged. */
struct GORGEOUSCORERUNTIME_API FGorgeousVDGConsent
{
    static AGorgeousPlayerController* ResolveController(const UObject* Context);
    static bool IsCollectionRequest(FName Key);
    static bool CanSendCollectionRequest(const UObject* Context);
    static bool IsBundledHandler(const UObject* Object,FName Handler);
    static void RegisterBundledHandlers(UObject* Object);
    static bool ValidateInvocation(UObject* Object,const FGorgeousQueuedRPC& Request,const UFunction* Function,const void* Parameters);
};
