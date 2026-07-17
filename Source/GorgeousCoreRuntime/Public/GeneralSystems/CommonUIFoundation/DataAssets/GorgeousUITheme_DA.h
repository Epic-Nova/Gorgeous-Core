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
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include "Styling/SlateTypes.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIInstancedValueUtils.h"
#include "GeneralSystems/GorgeousPrimaryDataAsset.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousUITheme_DA.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous UITheme
| Functional Name: UGorgeousUITheme_DA
| Parent Class: UGorgeousPrimaryDataAsset
| Class Suffix: _DA
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Data Asset defining a global UI theme/skin. Uses Instanced Structs to
| allow overriding any property (Colors, Fonts, Sizes, etc.).
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(BlueprintType,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/DataAssets/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/DataAssets/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousUITheme_DA : public UGorgeousPrimaryDataAsset
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:
	// Identifies this asset as a UI theme primary asset.
	virtual FPrimaryAssetType GetPrimaryAssetType() const override { return TEXT("UI_Theme"); }

	// Returns the directories that contain UI theme assets.
	virtual TArray<FString> GetDefaultScanPaths() const override { return { TEXT("UserInterfaces") }; }
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:
	/**
	 * Returns the valid platform names for icon mapping.
	 *
	 * @return The supported platform names.
	 */
	UFUNCTION()
	static TArray<FString> GetPlatformOptions()
	{
		return { TEXT("Keyboard"), TEXT("Xbox"), TEXT("PlayStation"), TEXT("Switch"), TEXT("Mobile"), TEXT("Generic") };
	}

	/**
	 * Retrieves a themed color by property name.
	 *
	 * @param PropertyName The name of the color property.
	 * @param DefaultValue The value returned when the property is not configured.
	 * @return The configured color or DefaultValue when absent.
	 */
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

	/**
	 * Retrieves a themed float by property name.
	 *
	 * @param PropertyName The name of the float property.
	 * @param DefaultValue The value returned when the property is not configured.
	 * @return The configured float or DefaultValue when absent.
	 */
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

	/**
	 * Retrieves a themed brush by property name.
	 *
	 * @param PropertyName The name of the brush property.
	 * @return The configured brush or an empty brush when absent.
	 */
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
	 * Retrieves an action icon using the configured platform fallback order.
	 *
	 * @param ActionTag The action whose icon is requested.
	 * @param PlatformName The active platform name.
	 * @return The matching icon or an empty brush when no icon is configured.
	 */
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

			// 2. Generic Fallback
			if (const FSlateBrush* GenericIcon = IconGroup->PlatformIcons.Find(TEXT("Generic")))
			{
				return *GenericIcon;
			}

			// 3. PC fallback (Keyboard)
			if (const FSlateBrush* PCIcon = IconGroup->PlatformIcons.Find(TEXT("Keyboard")))
			{
				return *PCIcon;
			}
			// 4. Emergency Fallback: Take the first available icon in the map
			for (auto& Pair : IconGroup->PlatformIcons)
			{
				return Pair.Value;
			}
		}
		return FSlateBrush();
	}

	/**
	 * Retrieves a themed sound by tag.
	 *
	 * @param SoundTag The tag that identifies the sound.
	 * @return The configured sound, or null when no sound is configured.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI|Theme")
	USoundBase* GetThemedSound(FGameplayTag SoundTag) const
	{
		if (const TObjectPtr<USoundBase>* SoundPtr = SoundMap.Find(SoundTag))
		{
			return SoundPtr->Get();
		}
		return nullptr;
	}

	/**
	 * Retrieves typography settings by tag.
	 *
	 * @param Tag The tag that identifies the typography settings.
	 * @return The configured typography or a default value when absent.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Theme")
	FGorgeousUITypography_S GetTypography(FGameplayTag Tag) const
	{
		if (const FInstancedStruct* Value = StyleProperties.Find(Tag.GetTagName()))
		{
			if (const FGorgeousUITypography_S* TypographyPtr = Value->GetPtr<FGorgeousUITypography_S>())
			{
				return *TypographyPtr;
			}
		}
		return FGorgeousUITypography_S();
	}

	/**
	 * Retrieves a progress bar style by property name.
	 *
	 * @param PropertyName The name of the progress bar style property.
	 * @return The configured style or a default style when absent.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Theme")
	FProgressBarStyle GetProgressBarStyle(FName PropertyName) const
	{
		if (const FInstancedStruct* Value = StyleProperties.Find(PropertyName))
		{
			if (const FProgressBarStyle* StylePtr = Value->GetPtr<FProgressBarStyle>())
			{
				return *StylePtr;
			}
		}
		return FProgressBarStyle();
	}
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:
	// Maps style property names to their themed values.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Styles", meta = (ShowOnlyInnerProperties))
	TMap<FName, FInstancedStruct> StyleProperties;

	// Maps input action tags to platform-specific icon groups.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Icons", meta = (Categories = "UI.Action"))
	TMap<FGameplayTag, FGorgeousPlatformIconGroup_S> ActionIcons;

	// Maps gameplay tags to themed UI sounds.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TMap<FGameplayTag, TObjectPtr<USoundBase>> SoundMap;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};