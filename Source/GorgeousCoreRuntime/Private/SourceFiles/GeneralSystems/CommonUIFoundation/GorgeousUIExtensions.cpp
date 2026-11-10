// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/GorgeousUIExtensions.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "CommonInputSubsystem.h"
#include "CommonActivatableWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousPrimaryGameLayout.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousHUD.h"

AGorgeousHUD* UGorgeousUIExtensions::GetGorgeousHUD(const UObject* WorldContextObject)
{
	if (auto* Subsystem = UGorgeousUIFoundationSubsystem::GetForContext(WorldContextObject))
	{ if (auto* Player = Subsystem->GetLocalPlayer()) if (auto* Controller = Player->GetPlayerController(Subsystem->GetWorld())) return Cast<AGorgeousHUD>(Controller->GetHUD()); }
	return nullptr;
}

FSlateBrush UGorgeousUIExtensions::GetGorgeousActionIcon(UObject* WorldContextObject, FGameplayTag ActionTag)
{
	if (!WorldContextObject) return FSlateBrush();

	UGorgeousUIFoundationSubsystem* Subsystem = UGorgeousUIFoundationSubsystem::GetForContext(WorldContextObject);
	if (!Subsystem) return FSlateBrush();

	TArray<UGorgeousUITheme_DA*> CurrentThemes = Subsystem->GetCurrentThemes();
	if (CurrentThemes.IsEmpty()) return FSlateBrush();
	
	// Reverse iterate to fetch resources from the most recent theme first
	//@TODO: Since this reverse iterating code is used very often in the gorgeous ui context, we might want to write a template functor for this
	for (int32 i = Subsystem->GetCurrentThemes().Num() - 1; i >= 0; --i)
	{
		if (const FSlateBrush ThemeBrush = Subsystem->GetCurrentThemes()[i]->GetActionIcon(ActionTag, Subsystem->GetCurrentPlatformName()); 
			ThemeBrush.GetResourceName() != NAME_None)
		{
			return ThemeBrush;
		}
	}
	
	return FSlateBrush();
}

UCommonActivatableWidget* UGorgeousUIExtensions::PushGorgeousWidget(UObject* WorldContextObject, FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass, ULocalPlayer* LocalPlayer)
{
	if (!WorldContextObject || !WidgetClass) return nullptr;

	if (!LocalPlayer)
	{
		if (auto* Subsystem = UGorgeousUIFoundationSubsystem::GetForContext(WorldContextObject)) LocalPlayer = Subsystem->GetLocalPlayer();
	}

	if (UGorgeousPrimaryGameLayout* Layout = UGorgeousPrimaryGameLayout::GetPrimaryGameLayout(LocalPlayer))
	{
		return Layout->PushWidgetToLayerStack(LayerTag, WidgetClass);
	}

	return nullptr;
}

void UGorgeousUIExtensions::PushGorgeousWidgetAsync(UObject* WorldContextObject, FGameplayTag LayerTag, TSoftClassPtr<UCommonActivatableWidget> WidgetClass, bool bSuspendInput, ULocalPlayer* LocalPlayer)
{
	if (!WorldContextObject || WidgetClass.IsNull()) return;

	if (!LocalPlayer)
	{
		if (auto* Subsystem = UGorgeousUIFoundationSubsystem::GetForContext(WorldContextObject)) LocalPlayer = Subsystem->GetLocalPlayer();
	}

	if (UGorgeousPrimaryGameLayout* Layout = UGorgeousPrimaryGameLayout::GetPrimaryGameLayout(LocalPlayer))
	{
		Layout->PushWidgetToLayerStackAsync(LayerTag, bSuspendInput, WidgetClass);
	}
}

namespace
{
	TMap<TWeakObjectPtr<APlayerController>, TSet<FName>> GorgeousInputSuspensions;
}

FName UGorgeousUIExtensions::SuspendGorgeousInput(APlayerController* PlayerController, FName Reason)
{
	if (!IsValid(PlayerController)) return NAME_None;
	for (auto It = GorgeousInputSuspensions.CreateIterator(); It; ++It) if (!It.Key().IsValid()) It.RemoveCurrent();
	const FName Token(*FString::Printf(TEXT("%s_%s"), *Reason.ToString(), *FGuid::NewGuid().ToString()));
	GorgeousInputSuspensions.FindOrAdd(PlayerController).Add(Token);
	PlayerController->SetIgnoreMoveInput(true);
	PlayerController->SetIgnoreLookInput(true);
	return Token;
}

void UGorgeousUIExtensions::ResumeGorgeousInput(APlayerController* PlayerController, FName Token)
{
	if (!IsValid(PlayerController) || Token.IsNone()) return;
	auto* Tokens = GorgeousInputSuspensions.Find(PlayerController);
	if (!Tokens || !Tokens->Remove(Token)) return;
	if (Tokens->IsEmpty()) GorgeousInputSuspensions.Remove(PlayerController);
	PlayerController->SetIgnoreMoveInput(false);
	PlayerController->SetIgnoreLookInput(false);
}

float UGorgeousUIExtensions::CalculateEasedAlpha(float InAlpha, EGorgeousUIInterpType_E InterpType)
{
	InAlpha = FMath::Clamp(InAlpha, 0.0f, 1.0f);

	switch (InterpType)
	{
	case EGorgeousUIInterpType_E::Linear:
		return InAlpha;
	case EGorgeousUIInterpType_E::EaseIn:
		return InAlpha * InAlpha * InAlpha; // Cubic
	case EGorgeousUIInterpType_E::EaseOut:
		return 1.0f - FMath::Pow(1.0f - InAlpha, 3.0f); // Cubic
	case EGorgeousUIInterpType_E::EaseInOut:
		return InAlpha < 0.5f ? 4.0f * InAlpha * InAlpha * InAlpha : 1.0f - FMath::Pow(-2.0f * InAlpha + 2.0f, 3.0f) / 2.0f;
	}

	return InAlpha;
}
