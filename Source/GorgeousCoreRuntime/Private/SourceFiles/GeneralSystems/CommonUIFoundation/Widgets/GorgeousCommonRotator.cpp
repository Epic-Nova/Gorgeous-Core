// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonRotator.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIInstancedValueUtils.h"

void UGorgeousCommonRotator::NativeConstruct()
{
	UE_UI_REGISTER_WIDGET()

	if (BindingTag.IsValid())
	{
		FSignalBridgeEventDelegate Delegate;
		Delegate.BindDynamic(this, &UGorgeousCommonRotator::OnRotatorSignalReceived);
		
		FGorgeousSignalBridgeAccessRules_S Rules;
		AGorgeousPlayerController* Controller = nullptr;
		if (UWorld* __World = GetWorld())
		{
			if (APlayerController* PC = __World->GetFirstPlayerController())
			{
				Controller = Cast<AGorgeousPlayerController>(PC);
			}
		}
		USignalBridgeBlueprintFunctionLibrary::RegisterSignal(GetWorld(), BindingTag, Rules, Controller);
		USignalBridgeBlueprintFunctionLibrary::Listen(GetWorld(), BindingTag, Controller, Delegate);
	}
}

void UGorgeousCommonRotator::NativeDestruct()
{
	if (BindingTag.IsValid())
	{
		AGorgeousPlayerController* Controller = nullptr;
		if (UWorld* __World = GetWorld())
		{
			if (APlayerController* PC = __World->GetFirstPlayerController())
			{
				Controller = Cast<AGorgeousPlayerController>(PC);
			}
		}
		USignalBridgeBlueprintFunctionLibrary::Clear(GetWorld(), BindingTag, Controller);
	}

	UE_UI_UNREGISTER_WIDGET()
}

void UGorgeousCommonRotator::OnRotatorSignalReceived(const FInstancedStruct& Payload)
{
	int32 IndexOut = 0;
	if (GorgeousUIInstanced::TryGetInt(Payload, IndexOut))
	{
		SetSelectedItem(IndexOut);
	}
}

void UGorgeousCommonRotator::OnThemeApplied_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// Apply themed text styles or colors to internal components if needed.
}

void UGorgeousCommonRotator::OnThemeApplied_BP_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// Optional Blueprint hook for additional theme application logic.
}

UE_UI_IMPLEMENT_THEME_BRIDGE(UGorgeousCommonRotator)