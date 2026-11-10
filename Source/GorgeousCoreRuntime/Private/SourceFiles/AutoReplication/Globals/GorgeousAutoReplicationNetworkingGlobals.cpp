// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

#include "AutoReplication/Globals/GorgeousAutoReplicationNetworkingGlobals.h"
#include "GameFramework/PlayerState.h"
#include "Engine/LocalPlayer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

#include "GameFramework/PlayerController.h"
#include "HAL/PlatformProcess.h"

FString UGorgeousAutoReplicationNetworkingGlobals::MakeStablePlayerConnectionId(const APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return FString();
	}

    if (PlayerController->PlayerState && PlayerController->PlayerState->GetUniqueId().IsValid())
        return PlayerController->PlayerState->GetUniqueId()->ToString();
    const ULocalPlayer* Local = PlayerController->GetLocalPlayer();
    if (!Local) return FString(); // A server cannot manufacture a remote user's persistent identity.
    const int32 Slot = Local->GetControllerId();
    static TMap<int32,FString> Cache;
    if (const FString* Existing=Cache.Find(Slot)) return *Existing;
    const FString Filename=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Gorgeous/ConnectionIds"),FString::Printf(TEXT("LocalPlayer-%d.id"),Slot));
    FString Id;
    FGuid Parsed;
    if (!FFileHelper::LoadFileToString(Id,*Filename) || !FGuid::Parse(Id.TrimStartAndEnd(),Parsed))
    {
        Parsed=FGuid::NewGuid();
        Id=Parsed.ToString(EGuidFormats::DigitsWithHyphens);
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename),true);
        if (!FFileHelper::SaveStringToFile(Id,*Filename)) return FString();
    }
    Id=Parsed.ToString(EGuidFormats::DigitsWithHyphens);
    Cache.Add(Slot,Id);
    return Id;
}
