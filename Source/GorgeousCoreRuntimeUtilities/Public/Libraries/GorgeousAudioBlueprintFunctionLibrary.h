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
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousAudioBlueprintFunctionLibrary.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Delegates ---=================>
DECLARE_DYNAMIC_DELEGATE(FOnVoiceLineFinishedNative);
//<--------------------------------------------------->

UCLASS()
class GORGEOUSCORERUNTIMEUTILITIES_API UGorgeousAudioBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	//@TODO: This function needs to be completed with valid configuration providers (here to minimize the needed input parameters from the original function)
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Audio", meta = (WorldContext = "WorldContextObject"))
	static void PlayVoiceLineAtActorLocation(
		const TSoftObjectPtr<USoundBase> Sound,
		AActor* Actor,
		FOnVoiceLineFinishedNative OnVoiceLineFinished,
		UObject* WorldContextObject = nullptr)
	{
		if (!Sound.IsValid() || !Actor)
		{
			return;
		}
		
		Sound.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateLambda([=](const FSoftObjectPath& LoadedPath, UObject* LoadedObject)
		{
			if (!LoadedObject)
			{
				return;
			}

			if (UDialogueWave* DialogueSoundWave = Cast<UDialogueWave>(LoadedObject))
			{
				const FDialogueContext DialogueContext;
				//@TODO: Ask a runtime context provider to hand over a valid context
				//@e.g. the Actor can be used as the speaker and the actors in the near distance could be the listeners
				
				UGameplayStatics::PlayDialogueAtLocation(WorldContextObject, DialogueSoundWave, DialogueContext,
					Actor->GetActorLocation(),
					Actor->GetActorRotation(),
					1.f, 1.f, 0.f, nullptr);
				//@TODO: Get the float & attenuation values from a config provider (e.g. Entertaining Audio Config Provider which would be base of Core Audio Config Provider)
				OnVoiceLineFinished.ExecuteIfBound();
				return;
			}

			if (USoundBase* LoadedSound = Cast<USoundBase>(LoadedObject))
			{
				UGameplayStatics::PlaySoundAtLocation(WorldContextObject, LoadedSound, Actor->GetActorLocation());
				OnVoiceLineFinished.ExecuteIfBound();
			}
		}));
	}
};


using UGT_AudioFunctionLibrary = UGorgeousAudioBlueprintFunctionLibrary;