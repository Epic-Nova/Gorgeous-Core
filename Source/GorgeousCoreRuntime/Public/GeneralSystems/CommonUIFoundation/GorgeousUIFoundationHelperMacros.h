// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/Interfaces/GorgeousUIWidget_I.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
#include "Blueprint/UserWidget.h"

class UGorgeousUITheme_DA;

/**
 * Macros to reduce boilerplate in Gorgeous UI Foundation widgets.
 */

/**
 * Drops into the class declaration to implement IGorgeousUIWidget_I.
 * Implements GetBindingTag, GetAsWidget, and OnThemeApplied (as a BlueprintNativeEvent).
 */
#define UE_UI_WIDGET_INTERFACE_BOILERPLATE() \
public: \
	/* Return the binding tag stored on the object. */ \
	virtual FGameplayTag GetBindingTag() const override { return BindingTag; } \
	/* Helper to get a UUserWidget pointer when applicable; returns nullptr otherwise. */ \
	virtual UObject* GetAsWidget() override { return Cast<UObject>(this); } \
	/* Interface hook called when a theme is applied; concrete classes should implement this. */ \
	virtual void OnThemeApplied(const UGorgeousUITheme_DA* Theme) override; \
	/* C++ implementation hook that concrete classes may provide. */ \
	virtual void OnThemeApplied_Implementation(const UGorgeousUITheme_DA* Theme); \
	/* Optional Blueprint hook for theme application. */ \
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous UI", meta = (DisplayName = "On Theme Applied")) \
	void OnThemeApplied_BP(const UGorgeousUITheme_DA* Theme); \
	/* C++ hook for BlueprintNativeEvent default implementation (declare so CPP defs compile). */ \
	virtual void OnThemeApplied_BP_Implementation(const UGorgeousUITheme_DA* Theme); \
public:

/** Implement registration logic. To be used in NativeConstruct. */
#define UE_UI_REGISTER_WIDGET() \
	if (UWorld* World = GetWorld()) \
	{ \
		if (APlayerController* PC = World->GetFirstPlayerController()) \
		{ \
			if (ULocalPlayer* LP = PC->GetLocalPlayer()) \
			{ \
				if (UGorgeousUIFoundationSubsystem* UISubsystem = LP->GetSubsystem<UGorgeousUIFoundationSubsystem>()) \
				{ \
					UISubsystem->RegisterWidget(this); \
				} \
			} \
		} \
	}

/** Implement unregistration logic. To be used in NativeDestruct. */
#define UE_UI_UNREGISTER_WIDGET() \
	if (UWorld* World = GetWorld()) \
	{ \
		if (APlayerController* PC = World->GetFirstPlayerController()) \
		{ \
			if (ULocalPlayer* LP = PC->GetLocalPlayer()) \
			{ \
				if (UGorgeousUIFoundationSubsystem* UISubsystem = LP->GetSubsystem<UGorgeousUIFoundationSubsystem>()) \
				{ \
					UISubsystem->UnregisterWidget(this); \
				} \
			} \
		} \
	}

/** Implementation of theme color interpolation. To be used in NativeTick. */
#define UE_UI_TICK_THEME_INTERP() \
	if (bIsInterpTheme) \
	{ \
		bool bAnyChanged = false; \
		for (auto& Pair : TargetThemeColors) \
		{ \
			FLinearColor& Current = CurrentThemeColors.FindOrAdd(Pair.Key, Pair.Value); \
			if (!Current.Equals(Pair.Value)) \
			{ \
				Current = FMath::CInterpTo(Current, Pair.Value, InDeltaTime, ThemeInterpSpeed); \
				bAnyChanged = true; \
				FInstancedStruct ColorPayload = FInstancedStruct::Make(Current); \
				UGorgeousUIProcessor::ApplyPropertyToTarget(this, Pair.Key, ColorPayload); \
			} \
		} \
		if (!bAnyChanged) \
		{ \
			bIsInterpTheme = false; \
		} \
	}

	/** No-op define for legacy .cpp files that invoke lifecycle definition macros. */
	#define UE_UI_DEFINE_WIDGET_LIFECYCLE(Class)

	/** Declare lifecycle hooks that many widget .cpp files implement. Place this inside the UCLASS declaration when a .cpp defines these methods. */
	#define UE_UI_DECLARE_WIDGET_LIFECYCLE() \
	public: \
		virtual void NativeConstruct(); \
		virtual void NativeDestruct(); \
		virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime); \
		virtual void OnThemeApplied_Implementation(const UGorgeousUITheme_DA* Theme);

	/** Helper to resolve the UGorgeousUIFoundationSubsystem from any UObject context. */
	#define UE_UI_GET_LOCAL_PLAYER_SUBSYSTEM(OutVar) \
		UGorgeousUIFoundationSubsystem* OutVar = nullptr; \
		if (UWorld* __World = GetWorld()) \
		{ \
			if (APlayerController* __PC = __World->GetFirstPlayerController()) \
			{ \
				if (ULocalPlayer* __LP = __PC->GetLocalPlayer()) \
				{ \
					OutVar = __LP->GetSubsystem<UGorgeousUIFoundationSubsystem>(); \
				} \
			} \
		}

/** Implementation of the OnThemeApplied interface method. */
#define UE_UI_IMPLEMENT_THEME_BRIDGE(Class) \
	void Class::OnThemeApplied(const UGorgeousUITheme_DA* Theme) \
	{ \
		OnThemeApplied_Implementation(Theme); \
		OnThemeApplied_BP_Implementation(Theme); \
	}
