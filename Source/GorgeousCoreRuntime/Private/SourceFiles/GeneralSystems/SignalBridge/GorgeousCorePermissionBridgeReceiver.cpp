// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.

#include "GeneralSystems/SignalBridge/GorgeousCorePermissionBridgeReceiver.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"

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
	// TODO: Dev Settings GUI Integration
	// 1. Check if permission overrides exist in the graphical Dev Settings / config.
	// 2. If present, override and return that decision.

	// TODO: Team System Runtime Integration
	// 1. Query the Team/Guild of the Interactor actor.
	// 2. Lookup the role and associated permissions (e.g. check if role contains Request.PermissionName).
	// 3. Evaluate if permission is allowed based on target container ownership or faction rules.

	GT_I_LOG("GT.Core.Permission", TEXT("[PermissionBridgeReceiver] Evaluated permission '%s' for Interactor %s: GRANTED (Scaffold Default)"),
		*Request.PermissionName,
		Request.Interactor ? *Request.Interactor->GetName() : TEXT("NULL"));

	return true; // Default allowed in scaffold implementation
}
