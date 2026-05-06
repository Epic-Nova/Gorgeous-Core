// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIInstancedValueUtils.h"
#include "GorgeousUITheme_DA.generated.h"

/**
 * Data Asset defining a global UI theme/skin.
 * Uses Instanced Structs to allow overriding any property (Colors, Fonts, Sizes, etc.).
 */
UCLASS(BlueprintType)
class GORGEOUSCORERUNTIME_API UGorgeousUITheme_DA : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the list of valid platform names for icon mapping. */
	UFUNCTION()
	static TArray<FString> GetPlatformOptions()
	{
		return { TEXT("Keyboard"), TEXT("Xbox"), TEXT("PlayStation"), TEXT("Switch"), TEXT("Mobile"), TEXT("Generic") };
	}

	/** Map of style property names to their values. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Styles")
	TMap<FName, FInstancedStruct> StyleProperties;

	/** Helper to get a color by name. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Theme")
	FLinearColor GetColor(FName PropertyName, FLinearColor DefaultValue = FLinearColor::White) const
	{
		if (const FInstancedStruct* Value = StyleProperties.Find(PropertyName))
		{
			if (const FLinearColor* ColorPtr = Value->GetPtr<FLinearColor>())
			{
				return *ColorPtr;
			}
		}
		return DefaultValue;
	}

	/** Helper to get a float by name. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Theme")
	float GetFloat(FName PropertyName, float DefaultValue = 0.0f) const
	{
		if (const FInstancedStruct* Value = StyleProperties.Find(PropertyName))
		{
			float Out = 0.0f;
			if (GorgeousUIInstanced::TryGetFloat(*Value, Out))
			{
				return Out;
			}
		}
		return DefaultValue;
	}

	/** Helper to get a brush by name. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Theme")
	FSlateBrush GetBrush(FName PropertyName) const
	{
		if (const FInstancedStruct* Value = StyleProperties.Find(PropertyName))
		{
			if (const FSlateBrush* BrushPtr = Value->GetPtr<FSlateBrush>())
			{
				return *BrushPtr;
			}
		}
		return FSlateBrush();
	}

	/** 
	 * Icons for specific input actions (e.g. UI.Action.Confirm). 
	 * These are automatically swapped based on the active platform and theme.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Icons")
	TMap<FGameplayTag, FGorgeousPlatformIconGroup_S> ActionIcons;

	/** Helper to get an icon for an action and platform with prioritized fallbacks. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Theme")
	FSlateBrush GetActionIcon(FGameplayTag ActionTag, FName PlatformName) const
	{
		if (const FGorgeousPlatformIconGroup_S* IconGroup = ActionIcons.Find(ActionTag))
		{
			// 1. Direct Match (e.g. PlayStation)
			if (const FSlateBrush* Icon = IconGroup->PlatformIcons.Find(PlatformName))
			{
				return *Icon;
			}
			
			// 2. PC Fallback (Keyboard)
			if (const FSlateBrush* PCIcon = IconGroup->PlatformIcons.Find(TEXT("Keyboard")))
			{
				return *PCIcon;
			}
			
			// 3. Generic Fallback
			if (const FSlateBrush* GenericIcon = IconGroup->PlatformIcons.Find(TEXT("Generic")))
			{
				return *GenericIcon;
			}

			// 4. Emergency Fallback: Take the first available icon in the map
			for (auto& Pair : IconGroup->PlatformIcons)
			{
				return Pair.Value;
			}
		}
		return FSlateBrush();
	}

	/** Map of Gameplay Tags to UI Sounds for themed audio. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TMap<FGameplayTag, TObjectPtr<USoundBase>> SoundMap;

	/** Returns a themed sound for the given tag. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI|Theme")
	USoundBase* GetThemedSound(FGameplayTag SoundTag) const
	{
		if (const TObjectPtr<USoundBase>* SoundPtr = SoundMap.Find(SoundTag))
		{
			return SoundPtr->Get();
		}
		return nullptr;
	}
};
