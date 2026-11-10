// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "GeneralSystems/SignalBridge/GorgeousCorePermissionBridgeReceiver.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "GeneralSystems/GorgeousPermissionManagerSubsystem.h"
#include "GeneralSystems/GorgeousPermissionProvider.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UGorgeousCorePermissionBridgeReceiver::UGorgeousCorePermissionBridgeReceiver()
{
}

void UGorgeousCorePermissionBridgeReceiver::InitializeReceiver(UObject* WorldContextObject)
{
	SetFallbackOwner(WorldContextObject);
	if (!WorldContextObject)
	{
		return;
	}

	FGameplayTag RequestTag = FGameplayTag::RequestGameplayTag(TEXT("GT.SignalBridge.Permission.Request"));

	// Configure access rules for the permission signal.
	FGorgeousSignalBridgeAccessRules_S Rules;
	Rules.bNetworked = true;
	Rules.AccessPolicy = EGorgeousObjectVariableAccessPolicy::Everyone;
	
	USignalBridgeBlueprintFunctionLibrary::RegisterSignal(this, RequestTag, Rules);

	// Bind listener to permission check requests.
	FSignalBridgeEventDelegate Delegate;
	Delegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UGorgeousCorePermissionBridgeReceiver, OnReceivePermissionRequest));
	USignalBridgeBlueprintFunctionLibrary::Listen(this, RequestTag, nullptr, Delegate);

	GT_I_LOG("GT.Core.Permission", TEXT("[PermissionBridgeReceiver] Successfully initialized and listening to GT.SignalBridge.Permission.Request."));
}

void UGorgeousCorePermissionBridgeReceiver::OnReceivePermissionRequest(FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	if (!Payload.IsValid())
	{
		return;
	}

	const FGorgeousPermissionSignalPayload_S* RequestPayload = Payload.GetPtr<FGorgeousPermissionSignalPayload_S>();
	if (!RequestPayload)
	{
		return;
	}

	FGorgeousPermissionSignalPayload_S Response = *RequestPayload;
	Response.bGranted = EvaluatePermissionRequest(Response);

	// Send decision back via the requested response tag
	if (Response.ResponseTag.IsValid())
	{
		USignalBridgeBlueprintFunctionLibrary::Dispatch(this, Response.ResponseTag, FInstancedStruct::Make(Response));
	}
}

bool UGorgeousCorePermissionBridgeReceiver::EvaluatePermissionRequest(const FGorgeousPermissionSignalPayload_S& Request) const
{
    if (!Request.Interactor || !Request.Interactor->HasAuthority()) return false;
    UWorld* World = Request.Interactor->GetWorld();
    if (!World || World->GetNetMode() == NM_Client) return false;
    if (UGameInstance* GI = World->GetGameInstance())
    {
        if (auto* Permissions = GI->GetSubsystem<UGorgeousPermissionManagerSubsystem>())
        {
            bool bHandled = false;
            const bool bAllowed = Permissions->EvaluatePlayerPermission(Request.Interactor, Request.PermissionName, bHandled);
            if (bHandled) return bAllowed;
        }
    }
    if (auto* Provider = UGorgeousPermissionProvider::FindProvider(Request.PermissionName))
        return Provider->EvaluatePermission(Request.Interactor, nullptr);
    return false;
}
