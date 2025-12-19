// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

#include "ModuleCore/GorgeousAutoReplicationNetworkingLibrary.h"

#include "GameFramework/PlayerController.h"
#include "HAL/PlatformProcess.h"

namespace GorgeousAutoReplicationNetworkingLibrary_Private
{
static FString BuildMachineFingerprint()
{
	const FString OperatingSystemId = FPlatformMisc::GetOperatingSystemId();
	const FString LoginId = FPlatformMisc::GetLoginId();
	const FString ComputerName = FPlatformProcess::ComputerName();

	FString Fingerprint;
	if (!OperatingSystemId.IsEmpty())
	{
		Fingerprint = OperatingSystemId;
	}
	if (!LoginId.IsEmpty())
	{
		Fingerprint = Fingerprint.IsEmpty() ? LoginId : FString::Printf(TEXT("%s|%s"), *Fingerprint, *LoginId);
	}
	if (!ComputerName.IsEmpty())
	{
		Fingerprint = Fingerprint.IsEmpty() ? ComputerName : FString::Printf(TEXT("%s|%s"), *Fingerprint, *ComputerName);
	}

	if (Fingerprint.IsEmpty())
	{
		Fingerprint = FString::Printf(TEXT("Fallback-%llu"), FPlatformTime::Cycles64());
	}

	return Fingerprint;
}

static FString GetCachedMachineFingerprint()
{
	static FString CachedFingerprint = BuildMachineFingerprint();
	return CachedFingerprint;
}
}

FString UGorgeousAutoReplicationNetworkingLibrary::MakeStablePlayerConnectionId(const APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return FString();
	}

	const FGorgeousAutoReplicationRPCResponderHandle Handle = FGorgeousAutoReplicationRPCResponderHandle::FromController(PlayerController);
	const FString StableKey = Handle.GetStableKey();
	const FString ControllerKey = StableKey.IsEmpty() ? PlayerController->GetName() : StableKey;
	const FString MachineFingerprint = GorgeousAutoReplicationNetworkingLibrary_Private::GetCachedMachineFingerprint();
	return FString::Printf(TEXT("%s|%s"), *MachineFingerprint, *ControllerKey);
}
