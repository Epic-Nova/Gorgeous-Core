// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperMacros.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
#include "Components/Widget.h"
#include "Blueprint/UserWidget.h"
#include "Curves/CurveFloat.h"

/**
 * IMPLEMENTATION bodies for Gorgeous UI Foundation macros.
 * Include this ONLY in .cpp files to avoid circular dependencies with the Subsystem.
 */

#if WITH_EDITOR
#define UE_UI_WIDGET_EDITOR_IMPLEMENTATION(Class) \
	void Class::ApplyEditorThemeIfNeeded() \
	{ \
		UGorgeousUIProcessor::ApplyEditorThemeToWidget(this); \
	} \
	void Class::PostLoad() \
	{ \
		Super::PostLoad(); \
		ApplyEditorThemeIfNeeded(); \
	} \
	void Class::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) \
	{ \
		Super::PostEditChangeProperty(PropertyChangedEvent); \
		ApplyEditorThemeIfNeeded(); \
	} \
	void Class::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) \
	{ \
		Super::PostEditChangeChainProperty(PropertyChangedEvent); \
		ApplyEditorThemeIfNeeded(); \
	}
#else
#define UE_UI_WIDGET_EDITOR_IMPLEMENTATION(Class) \
	void Class::ApplyEditorThemeIfNeeded() {}
#endif

#define UE_UI_IMPLEMENT_WIDGET_INTERFACE(Class) \
	void Class::OnThemeApplied(const UGorgeousUITheme_DA* Theme) \
	{ \
		ApplyThemeInterpolation(Theme); \
		OnThemeApplied_BP(Theme); \
	} \
	void Class::OnThemeApplied_BP_Implementation(const UGorgeousUITheme_DA* Theme) {} \
	void Class::NotifyReadyForStateSwap() \
	{ \
		if (UWorld* __World = GetWorld()) \
		{ \
			if (APlayerController* __PC = __World->GetFirstPlayerController()) \
			{ \
				if (ULocalPlayer* __LP = __PC->GetLocalPlayer()) \
				{ \
					if (UGorgeousUIFoundationSubsystem* __Subsystem = __LP->GetSubsystem<UGorgeousUIFoundationSubsystem>()) \
					{ \
						__Subsystem->NotifyWidgetTransitionComplete(this); \
					} \
				} \
			} \
		} \
	} \
	void Class::ApplyOverlayConfig(const FGorgeousUIStateConfig& Config) \
	{ \
		if (UWidget* __Widget = Cast<UWidget>(this)) \
		{ \
			__Widget->SetVisibility(Config.bIsVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed); \
			this->StartOpacity = __Widget->GetRenderOpacity(); \
			this->TargetOpacity = Config.Opacity; \
			this->OpacityInterpConfig = Config.OpacityInterp; \
			this->ElapsedOpacityTime = 0.0f; \
			this->bIsInterpOpacity = Config.bEnableOpacityInterp; \
			if (!this->bIsInterpOpacity) __Widget->SetRenderOpacity(this->TargetOpacity); \
			\
			this->ThemeInterpConfig = Config.OpacityInterp; /* TODO: Add ThemeInterpConfig to StateConfig */ \
			this->ElapsedThemeTime = 0.0f; \
			this->bIsInterpTheme = Config.bEnableThemeInterp; \
		} \
	} \
	UE_UI_WIDGET_EDITOR_IMPLEMENTATION(Class) \
	\
	void Class::TickInterpolation_Implementation(float DeltaTime) \
	{ \
		auto __CalcInterpAlpha = [](float InAlpha, EGorgeousUIInterpType_E InType, const FRuntimeFloatCurve& InCurve) -> float \
		{ \
			const FRichCurve* RichCurve = const_cast<FRuntimeFloatCurve&>(InCurve).GetRichCurve(); \
			if (RichCurve && RichCurve->GetNumKeys() > 0) return RichCurve->Eval(InAlpha); \
			switch (InType) \
			{ \
				case EGorgeousUIInterpType_E::EaseIn: return InAlpha * InAlpha; \
				case EGorgeousUIInterpType_E::EaseOut: return 1.0f - (1.0f - InAlpha) * (1.0f - InAlpha); \
				case EGorgeousUIInterpType_E::EaseInOut: return InAlpha < 0.5f ? 2.0f * InAlpha * InAlpha : 1.0f - FMath::Pow(-2.0f * InAlpha + 2.0f, 2.0f) / 2.0f; \
				default: return InAlpha; \
			} \
		}; \
		\
		/* 1. Opacity Interpolation */ \
		if (this->bIsInterpOpacity) \
		{ \
			this->ElapsedOpacityTime += DeltaTime; \
			float Alpha = FMath::Clamp(this->ElapsedOpacityTime / FMath::Max(0.001f, this->OpacityInterpConfig.Duration), 0.0f, 1.0f); \
			float CurveValue = __CalcInterpAlpha(Alpha, this->OpacityInterpConfig.InterpType, this->OpacityInterpConfig.InterpCurve); \
			float NewOpacity = FMath::Lerp(this->StartOpacity, this->TargetOpacity, CurveValue); \
			this->SetWidgetOpacity(NewOpacity); \
			if (Alpha >= 1.0f) this->bIsInterpOpacity = false; \
		} \
		\
		/* 2. Theme Color Interpolation */ \
		if (this->bIsInterpTheme) \
		{ \
			this->ElapsedThemeTime += DeltaTime; \
			float Alpha = FMath::Clamp(this->ElapsedThemeTime / FMath::Max(0.001f, this->ThemeInterpConfig.Duration), 0.0f, 1.0f); \
			float CurveValue = __CalcInterpAlpha(Alpha, this->ThemeInterpConfig.InterpType, this->ThemeInterpConfig.InterpCurve); \
			for (auto& Pair : this->GetTargetThemeColors()) \
			{ \
				FLinearColor StartColor = this->GetStartThemeColors().FindRef(Pair.Key); \
				FLinearColor TargetColor = Pair.Value; \
				this->GetCurrentThemeColors().FindOrAdd(Pair.Key) = FMath::Lerp(StartColor, TargetColor, CurveValue); \
			} \
			this->ApplyThemeInterpolation(nullptr); \
			if (Alpha >= 1.0f) this->bIsInterpTheme = false; \
		} \
	}

#define UE_UI_IMPLEMENT_THEME_BRIDGE(Class) \
	void Class::OnThemeApplied(const UGorgeousUITheme_DA* Theme) \
	{ \
		OnThemeApplied_BP(Theme); \
	} \
	void Class::NotifyReadyForStateSwap() \
	{ \
		if (UWorld* __World = GetWorld()) \
		{ \
			if (APlayerController* __PC = __World->GetFirstPlayerController()) \
			{ \
				if (ULocalPlayer* __LP = __PC->GetLocalPlayer()) \
				{ \
					if (UGorgeousUIFoundationSubsystem* __Subsystem = __LP->GetSubsystem<UGorgeousUIFoundationSubsystem>()) \
					{ \
						__Subsystem->NotifyWidgetTransitionComplete(this); \
					} \
				} \
			} \
		} \
	}
