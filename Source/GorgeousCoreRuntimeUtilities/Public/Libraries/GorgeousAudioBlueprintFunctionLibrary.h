// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "Kismet/GameplayStatics.h"
#include "Sound/DialogueWave.h"
#include "Sound/SoundBase.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/SoftObjectPtr.h"
//<--------------------------=== Module Includes ===------------------------->
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousAudioBlueprintFunctionLibrary.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Delegates ---=================>
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
	 * @TODO: This function needs to be completed with valid configuration providers (here to minimize the needed input parameters from the original function)
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

		auto HandleLoadedObject = [=](UObject* LoadedObject)
		{
			UAudioComponent* Audio = nullptr;
			
			if (!LoadedObject)
			{	
				return Audio;
			}

			if (UDialogueWave* DialogueSoundWave = Cast<UDialogueWave>(LoadedObject))
			{
				const FDialogueContext DialogueContext;
				//@TODO: Ask a runtime context provider to hand over a valid context
				//@e.g. the Actor can be used as the speaker and the actors in the near distance could be the listeners

				Audio = UGameplayStatics::SpawnDialogueAtLocation(WorldContextObject, DialogueSoundWave, DialogueContext,
					Actor->GetActorLocation(),
					Actor->GetActorRotation(),
					1.f, 1.f, 0.f, nullptr);
				//@TODO: Get the float & attenuation values from a config provider (e.g. Entertaining Audio Config Provider which would be base of Core Audio Config Provider)
				OnVoiceLineAudioReady.ExecuteIfBound(Audio);
				OnVoiceLineFinished.ExecuteIfBound();
			}

			if (USoundBase* LoadedSound = Cast<USoundBase>(LoadedObject))
			{
				Audio = UGameplayStatics::SpawnSoundAtLocation(WorldContextObject, LoadedSound, Actor->GetActorLocation());
				OnVoiceLineAudioReady.ExecuteIfBound(Audio);
				OnVoiceLineFinished.ExecuteIfBound();
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
};

using UGT_Audio_FL = UGorgeousAudioBlueprintFunctionLibrary;