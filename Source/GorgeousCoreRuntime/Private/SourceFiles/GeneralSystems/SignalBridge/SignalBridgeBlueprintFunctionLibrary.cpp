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

	UGorgeousObjectVariable* ExistingVar = nullptr;
	if (UGorgeousCoreRuntimeGlobals::GetNetGorgeousAutoReplicationValue(WorldContextObject, SignalBridgeEntryKey, ExistingVar))
	{
		if (USignalBridgeStorage_OV* Storage = Cast<USignalBridgeStorage_OV>(ExistingVar))
		{
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

	return nullptr; 
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

void USignalBridgeBlueprintFunctionLibrary::Dispatch(UObject* WorldContextObject, FGameplayTag Tag, const FInstancedStruct& Payload)
{
	if (USignalBridgeStorage_OV* Storage = GetSignalBridgeStorage(WorldContextObject))
	{
		Storage->Dispatch(Tag, Payload);
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
