// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "GeneralSystems/SignalBridge/SignalBridgeStorage_OV.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "QualityOfLife/GorgeousWorldSettings.h"
#include "Engine/World.h"

static const FName SignalBridgeEntryKey = TEXT("SignalBridge");

USignalBridgeStorage_OV* USignalBridgeBlueprintFunctionLibrary::GetSignalBridgeStorage(UObject* WorldContextObject, bool bRequireNetworking)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	GT_I_LOG("GT.SignalBridge", TEXT("Retrieving Signal Bridge for world %s (Context: %s)"), *World->GetName(), *GetNameSafe(WorldContextObject));

	UGorgeousObjectVariable* ExistingVar = nullptr;
	if (UGorgeousCoreRuntimeGlobals::GetNetGorgeousAutoReplicationValue(WorldContextObject, SignalBridgeEntryKey, ExistingVar))
	{
		if (USignalBridgeStorage_OV* Storage = Cast<USignalBridgeStorage_OV>(ExistingVar))
		{
			GT_I_LOG("GT.SignalBridge", TEXT("Found Signal Bridge via AutoReplication: %s"), *Storage->GetName());
			return Storage;
		}
	}

	if (bRequireNetworking)
	{
		AGorgeousWorldSettings* WorldSettings = Cast<AGorgeousWorldSettings>(World->GetWorldSettings());
		if (WorldSettings)
		{
			if (!WorldSettings->AdditionalGorgeousData.Contains(SignalBridgeEntryKey))
			{
				if (WorldSettings->RegisterAutoReplicationEntry(SignalBridgeEntryKey, USignalBridgeStorage_OV::StaticClass(), true, false, FGorgeousAutoReplicationStreamConfig()))
				{
					UGorgeousCoreRuntimeGlobals::RefreshQualityOfLifeReplication(WorldContextObject, AGorgeousWorldSettings::StaticClass());

					if (UGorgeousCoreRuntimeGlobals::GetNetGorgeousAutoReplicationValue(WorldContextObject, SignalBridgeEntryKey, ExistingVar))
					{
						return Cast<USignalBridgeStorage_OV>(ExistingVar);
					}
				}
			}
		}
	}

	// FALLBACK: If we are here, AutoReplication failed to resolve (likely misconfigured level settings).
	// We'll look for a transient storage attached to the world as a local-only fallback.
	USignalBridgeStorage_OV* LocalStorage = nullptr;
	for (TObjectIterator<USignalBridgeStorage_OV> It; It; ++It)
	{
		if (It->GetVariableWorld() == World && !It->IsTemplate())
		{
			LocalStorage = *It;
			break;
		}
	}

	if (!LocalStorage)
	{
		GT_W_LOG("GT.SignalBridge", TEXT("Signal Bridge: AutoReplication failed and no local storage found. Creating transient fallback for world %s."), *World->GetName());
		LocalStorage = NewObject<USignalBridgeStorage_OV>(World);
	}

	GT_I_LOG("GT.SignalBridge", TEXT("Returning Signal Bridge: %s"), *LocalStorage->GetName());
	return LocalStorage; 
}

void USignalBridgeBlueprintFunctionLibrary::RegisterSignal(UObject* WorldContextObject, FGameplayTag Tag, const FGorgeousSignalBridgeAccessRules_S& Rules, AGorgeousPlayerController* Requester)
{
	if (USignalBridgeStorage_OV* Storage = GetSignalBridgeStorage(WorldContextObject))
	{
		Storage->RegisterSignal(Tag, Rules, Requester);
	}
}

bool USignalBridgeBlueprintFunctionLibrary::Listen(UObject* WorldContextObject, FGameplayTag Tag, AGorgeousPlayerController* Controller, const FSignalBridgeEventDelegate& Delegate)
{
	if (USignalBridgeStorage_OV* Storage = GetSignalBridgeStorage(WorldContextObject))
	{
		return Storage->Listen(Tag, Controller, Delegate);
	}
	return false;
}

bool USignalBridgeBlueprintFunctionLibrary::ListenToActor(UObject* WorldContextObject, FGameplayTag Tag, AActor* TargetActor, AGorgeousPlayerController* Controller, const FSignalBridgeEventDelegate& Delegate)
{
	if (USignalBridgeStorage_OV* Storage = GetSignalBridgeStorage(WorldContextObject))
	{
		return Storage->ListenToActor(Tag, TargetActor, Controller, Delegate);
	}
	return false;
}

void USignalBridgeBlueprintFunctionLibrary::Dispatch(UObject* WorldContextObject, FGameplayTag Tag, const FInstancedStruct& Payload)
{
	if (USignalBridgeStorage_OV* Storage = GetSignalBridgeStorage(WorldContextObject))
	{
		Storage->Dispatch(Tag, Payload);
	}
}

void USignalBridgeBlueprintFunctionLibrary::DispatchLocal(UObject* WorldContextObject, FGameplayTag Tag, const FInstancedStruct& Payload)
{
	//@TODO: Seems not to work
	if (USignalBridgeStorage_OV* Storage = GetSignalBridgeStorage(WorldContextObject))
	{
		Storage->FireLocalSignal(Tag, Payload);
	}
}

void USignalBridgeBlueprintFunctionLibrary::Clear(UObject* WorldContextObject, FGameplayTag Tag, AGorgeousPlayerController* Controller)
{
	if (USignalBridgeStorage_OV* Storage = GetSignalBridgeStorage(WorldContextObject))
	{
		Storage->Clear(Tag, Controller);
	}
}

void USignalBridgeBlueprintFunctionLibrary::AddAllowedController(UObject* WorldContextObject, FGameplayTag Tag, AGorgeousPlayerController* Controller)
{
	if (USignalBridgeStorage_OV* Storage = GetSignalBridgeStorage(WorldContextObject))
	{
		Storage->AddAllowedController(Tag, Controller);
	}
}

void USignalBridgeBlueprintFunctionLibrary::RemoveAllowedController(UObject* WorldContextObject, FGameplayTag Tag, AGorgeousPlayerController* Controller)
{
	if (USignalBridgeStorage_OV* Storage = GetSignalBridgeStorage(WorldContextObject))
	{
		Storage->RemoveAllowedController(Tag, Controller);
	}
}
