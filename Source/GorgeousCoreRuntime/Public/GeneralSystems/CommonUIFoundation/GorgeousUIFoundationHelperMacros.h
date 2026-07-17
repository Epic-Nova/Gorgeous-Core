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
#include "GeneralSystems/CommonUIFoundation/Interfaces/GorgeousUIWidget_I.h"
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIOverlayConfig_DA.h"
//<-------------------------------------------------------------------------->

#if WITH_EDITOR
struct FPropertyChangedChainEvent;
struct FPropertyChangedEvent;
#endif

class UGorgeousUITheme_DA;
class UGorgeousUIFoundationSubsystem;

/**
 * Macros to reduce boilerplate in Gorgeous UI Foundation widgets.
 */

#if WITH_EDITOR
#define UE_UI_WIDGET_EDITOR_MEMBERS() \
	virtual void PostLoad() override; \
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override; \
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#else
#define UE_UI_WIDGET_EDITOR_MEMBERS()
#endif

/**
 * Drops into the class declaration to implement IGorgeousUIWidget_I.
 * Implements GetBindingTag, GetAsWidget, and OnThemeApplied (as a BlueprintNativeEvent).
 */
#define UE_UI_WIDGET_INTERFACE_BOILERPLATE() \
protected: \
	float StartOpacity = 1.0f; \
	float TargetOpacity = 1.0f; \
	float ElapsedOpacityTime = 0.0f; \
	FGorgeousUIInterpConfig_S OpacityInterpConfig; \
	bool bIsInterpOpacity = false; \
	\
	float ElapsedThemeTime = 0.0f; \
	FGorgeousUIInterpConfig_S ThemeInterpConfig; \
	TMap<FName, FLinearColor> StartThemeColors; \
	TMap<FName, FLinearColor> TargetThemeColors; \
	TMap<FName, FLinearColor> CurrentThemeColors; \
	bool bIsInterpTheme = false; \
	\
public: \
	virtual UObject* GetAsWidget() override { return (UObject*)this; } \
	virtual float GetWidgetOpacity() const override { if (const UWidget* __W = (const UWidget*)this) return __W->GetRenderOpacity(); return 1.0f; } \
	virtual void SetWidgetOpacity(float InOpacity) override { if (UWidget* __W = (UWidget*)this) __W->SetRenderOpacity(InOpacity); } \
	virtual bool UseStylePropertyAllowList() const override { return bUseStylePropertyAllowList; } \
	virtual const TSet<FName>& GetStylePropertyAllowList() const override { return StylePropertyAllowList; } \
	virtual float GetTargetOpacity() const override { return TargetOpacity; } \
	virtual float GetStartOpacity() const override { return StartOpacity; } \
	virtual FGorgeousUIInterpConfig_S GetOpacityInterpConfig() const override { return OpacityInterpConfig; } \
	virtual float GetElapsedOpacityTime() const override { return ElapsedOpacityTime; } \
	virtual void SetElapsedOpacityTime(float InTime) override { ElapsedOpacityTime = InTime; } \
	virtual bool IsInterpOpacity() const override { return bIsInterpOpacity; } \
	virtual void SetIsInterpOpacity(bool bInInterp) override { bIsInterpOpacity = bInInterp; } \
	virtual float GetElapsedThemeTime() const override { return ElapsedThemeTime; } \
	virtual void SetElapsedThemeTime(float InTime) override { ElapsedThemeTime = InTime; } \
	virtual bool IsInterpTheme() const override { return bIsInterpTheme; } \
	virtual void SetIsInterpTheme(bool bInInterp) override { bIsInterpTheme = bInInterp; } \
	virtual FGorgeousUIInterpConfig_S GetThemeInterpConfig() const override { return ThemeInterpConfig; } \
	virtual TMap<FName, FLinearColor>& GetStartThemeColors() override { return StartThemeColors; } \
	virtual TMap<FName, FLinearColor>& GetTargetThemeColors() override { return TargetThemeColors; } \
	virtual TMap<FName, FLinearColor>& GetCurrentThemeColors() override { return CurrentThemeColors; } \
	virtual FGameplayTag GetBindingTag() const override { return BindingTag; } \
	virtual void NotifyReadyForStateSwap() override; \
	virtual void OnThemeApplied(const UGorgeousUITheme_DA* Theme) override; \
	virtual void ApplyThemeInterpolation(const UGorgeousUITheme_DA* Theme); \
	virtual void TickInterpolation_Implementation(float DeltaTime) override; \
	\
	/* Applies a state-specific configuration (visibility, opacity, animation) to this widget. */ \
	virtual void ApplyOverlayConfig(const FGorgeousUIStateConfig& Config) override; \
	\
	void ApplyEditorThemeIfNeeded(); \
	UE_UI_WIDGET_EDITOR_MEMBERS() \
public:

/** Registration logic for standard UWidgets (Borders, Boxes). Use in SynchronizeProperties. */
#define UE_UI_REGISTER_WIDGET_RAW() \
	if (UWorld* __World = GetWorld()) \
	{ \
		if (APlayerController* __PC = __World->GetFirstPlayerController()) \
		{ \
			if (ULocalPlayer* __LP = __PC->GetLocalPlayer()) \
			{ \
				if (UGorgeousUIFoundationSubsystem* __Subsystem = __LP->GetSubsystem<UGorgeousUIFoundationSubsystem>()) \
				{ \
					__Subsystem->RegisterWidget(this); \
				} \
			} \
		} \
	} \
	\
	/* Editor-time theme evaluation */ \
	ApplyEditorThemeIfNeeded();

/** Registration logic for UUserWidgets. Use in NativeConstruct. */
#define UE_UI_REGISTER_WIDGET_USER() \
	UE_UI_REGISTER_WIDGET_RAW() \
	StartOpacity = TargetOpacity = GetRenderOpacity();

/** Unregistration logic. Use in NativeDestruct or OnWidgetRebuilt(false). */
#define UE_UI_UNREGISTER_WIDGET() \
	if (UWorld* __World = GetWorld()) \
	{ \
		if (APlayerController* __PC = __World->GetFirstPlayerController()) \
		{ \
			if (ULocalPlayer* __LP = __PC->GetLocalPlayer()) \
			{ \
				if (UGorgeousUIFoundationSubsystem* __Subsystem = __LP->GetSubsystem<UGorgeousUIFoundationSubsystem>()) \
				{ \
					__Subsystem->UnregisterWidget(this); \
				} \
			} \
		} \
	}

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