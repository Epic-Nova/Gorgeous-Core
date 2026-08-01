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
	if (const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			return Cast<AGorgeousHUD>(PC->GetHUD());
		}
	}
	return nullptr;
}

FSlateBrush UGorgeousUIExtensions::GetGorgeousActionIcon(UObject* WorldContextObject, FGameplayTag ActionTag)
{
	if (!WorldContextObject) return FSlateBrush();

	// Get the local player and query the LocalPlayer subsystem (UGorgeousUIFoundationSubsystem derives from ULocalPlayerSubsystem)
	ULocalPlayer* LocalPlayer = nullptr;
	if (UWorld* World = WorldContextObject->GetWorld())
	{
		LocalPlayer = World->GetFirstLocalPlayerFromController();
	}
	if (!LocalPlayer) return FSlateBrush();

	UGorgeousUIFoundationSubsystem* Subsystem = LocalPlayer->GetSubsystem<UGorgeousUIFoundationSubsystem>();
	if (!Subsystem) return FSlateBrush();

	TArray<UGorgeousUITheme_DA*> CurrentThemes = Subsystem->GetCurrentThemes();
	if (CurrentThemes.IsEmpty()) return FSlateBrush();
	
	// Reverse iterate to fetch resources from the most recent theme first
	//@TODO: Since this reverse iterating code is used very often in the gorgeous ui context, we might want to write a template functor for this
	for (int32 i = 0; i < Subsystem->GetCurrentThemes().Num() - 1; --i)
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
		LocalPlayer = WorldContextObject->GetWorld()->GetFirstLocalPlayerFromController();
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
		LocalPlayer = WorldContextObject->GetWorld()->GetFirstLocalPlayerFromController();
	}

	if (UGorgeousPrimaryGameLayout* Layout = UGorgeousPrimaryGameLayout::GetPrimaryGameLayout(LocalPlayer))
	{
		Layout->PushWidgetToLayerStackAsync(LayerTag, bSuspendInput, WidgetClass);
	}
}

//@TODO
FName UGorgeousUIExtensions::SuspendGorgeousInput(APlayerController* PlayerController, FName Reason)
{
	if (!PlayerController) return NAME_None;
	
	// In a AAA system, we would push a suspension token to a stack.
	// For now, we'll use a unique name.
	FName Token = FName(*FString::Printf(TEXT("%s_%f"), *Reason.ToString(), FPlatformTime::Seconds()));
	
	PlayerController->SetIgnoreMoveInput(true);
	PlayerController->SetIgnoreLookInput(true);
	
	return Token;
}

//@TODO - implement a stack of suspension tokens to allow multiple overlapping input suspensions (e.g. during nested menu navigation)
void UGorgeousUIExtensions::ResumeGorgeousInput(APlayerController* PlayerController, FName Token)
{
	if (!PlayerController || Token.IsNone()) return;

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
