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
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Kismet/GameplayStatics.h"
#include "Sound/DialogueWave.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/SoftObjectPtr.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousAudioBlueprintFunctionLibrary.generated.h"
//<-------------------------------------------------------------------------->

DECLARE_DYNAMIC_DELEGATE(FOnVoiceLineFinishedNative);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnVoiceLineAudioReadyNative, UAudioComponent*, AudioComponent);
//<--------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Audio Blueprint Function Library
| Functional Name: UGorgeousAudioBlueprintFunctionLibrary
| Parent Class: UBlueprintFunctionLibrary
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Function Library providing audio related helper functions for Blueprints.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS()
class GORGEOUSCORERUNTIMEUTILITIES_API UGorgeousAudioBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Plays a voice line sound at the location of the specified actor.
	 *
	 * @param Sound The soft object pointer to the sound asset to be played.
	 * @param Actor The actor at whose location the sound will be played.
	 * @param OnVoiceLineFinished A delegate that will be executed when the voice line has finished playing.
	 * @param OnVoiceLineAudioReady A delegate that will be executed when the audio component is spawned.
	 * @param WorldContextObject The world context object for locating the world in which to play the sound.
	 *
	 * @TODO: This function needs to be completed with valid configuration providers (here to minimize the needed input parameters from the original function) providers could also stream stuff from the www
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Audio", meta = (WorldContext = "WorldContextObject"))
	static void PlayVoiceLineAtActorLocation(
		const TSoftObjectPtr<USoundBase> Sound,
		AActor* Actor,
		FOnVoiceLineFinishedNative OnVoiceLineFinished,
		FOnVoiceLineAudioReadyNative OnVoiceLineAudioReady,
		UObject* WorldContextObject = nullptr)
	{
		if (Sound.IsNull() || !Actor)
		{
			return;
		}

		const TWeakObjectPtr<AActor> WeakActor(Actor);
		const TWeakObjectPtr<UObject> WeakWorldContext(WorldContextObject ? WorldContextObject : Actor);
		auto HandleLoadedObject = [=](UObject* LoadedObject)
		{
			UAudioComponent* Audio = nullptr;

			if (!LoadedObject || !WeakActor.IsValid() || !WeakWorldContext.IsValid())
			{
				return Audio;
			}


			if (USoundBase* LoadedSound = Cast<USoundBase>(LoadedObject))
			{
				Audio = UGameplayStatics::SpawnSoundAtLocation(WeakWorldContext.Get(), LoadedSound, WeakActor->GetActorLocation());
				if (Audio)
				{
					Audio->OnAudioFinishedNative.AddLambda([OnVoiceLineFinished](UAudioComponent*) { OnVoiceLineFinished.ExecuteIfBound(); });
				}
				OnVoiceLineAudioReady.ExecuteIfBound(Audio);
			}

			return Audio;
		};

		const FSoftObjectPath SoundPath = Sound.ToSoftObjectPath();

		GORGEOUS_55_HIGHER(
			Sound.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateLambda([=](const FSoftObjectPath& LoadedPath, UObject* LoadedObject)
			{
				HandleLoadedObject(LoadedObject);
			}));
		)

		GORGEOUS_54_LOWER(
			if (!SoundPath.IsValid())
			{
				return;
			}

			FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
			StreamableManager.RequestAsyncLoad(SoundPath, FStreamableDelegate::CreateLambda([=]()
			{
				UObject* LoadedObject = Sound.Get();
				if (!LoadedObject)
				{
					LoadedObject = SoundPath.ResolveObject();
				}
				if (!LoadedObject)
				{
					LoadedObject = Sound.LoadSynchronous();
				}
				HandleLoadedObject(LoadedObject);
			}));
		)
	}
    /** Dialogue-specific variant; the caller supplies the speaker/listener context. */
    UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Audio", meta = (WorldContext = "WorldContextObject"))
    static void PlayDialogueVoiceLineAtActorLocation(TSoftObjectPtr<UDialogueWave> Dialogue, const FDialogueContext& Context,
        AActor* Actor, FOnVoiceLineFinishedNative OnVoiceLineFinished, FOnVoiceLineAudioReadyNative OnVoiceLineAudioReady,
        UObject* WorldContextObject = nullptr)
    {
        if (Dialogue.IsNull() || !IsValid(Actor)) { return; }
        const TWeakObjectPtr<AActor> WeakActor(Actor);
        const TWeakObjectPtr<UObject> WeakWorldContext(WorldContextObject ? WorldContextObject : Actor);
        UAssetManager::GetStreamableManager().RequestAsyncLoad(Dialogue.ToSoftObjectPath(), FStreamableDelegate::CreateLambda(
            [Dialogue, Context, WeakActor, WeakWorldContext, OnVoiceLineFinished, OnVoiceLineAudioReady]
            {
                if (!Dialogue.IsValid() || !WeakActor.IsValid() || !WeakWorldContext.IsValid()) { return; }
                UAudioComponent* Audio = UGameplayStatics::SpawnDialogueAtLocation(WeakWorldContext.Get(), Dialogue.Get(), Context,
                    WeakActor->GetActorLocation(), WeakActor->GetActorRotation());
                if (Audio)
                {
                    Audio->OnAudioFinishedNative.AddLambda([OnVoiceLineFinished](UAudioComponent*) { OnVoiceLineFinished.ExecuteIfBound(); });
                }
                OnVoiceLineAudioReady.ExecuteIfBound(Audio);
            }));
    }

};

using UGT_Audio_FL = UGorgeousAudioBlueprintFunctionLibrary;