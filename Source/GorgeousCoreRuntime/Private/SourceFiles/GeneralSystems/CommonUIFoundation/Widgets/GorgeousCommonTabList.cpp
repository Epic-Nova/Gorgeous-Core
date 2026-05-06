// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonTabList.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIInstancedValueUtils.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "QualityOfLife/GorgeousPlayerController.h"

void UGorgeousCommonTabList::NativeConstruct()
{
	UE_UI_REGISTER_WIDGET()

	// Register for tab selection signals
	if (BindingTag.IsValid())
	{
		FSignalBridgeEventDelegate Delegate;
		Delegate.BindDynamic(this, &UGorgeousCommonTabList::OnTabSelectSignalReceived);

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

void UGorgeousCommonTabList::NativeDestruct()
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

void UGorgeousCommonTabList::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	UE_UI_TICK_THEME_INTERP()
}

UE_UI_IMPLEMENT_THEME_BRIDGE(UGorgeousCommonTabList)

void UGorgeousCommonTabList::OnTabSelectSignalReceived(const FInstancedStruct& Payload)
{
	// Handle TabID (FName)
	FName NameOut;
	if (GorgeousUIInstanced::TryGetName(Payload, NameOut))
	{
		SelectTabByID(NameOut);
		return;
	}

	// Handle TabIndex (int) - not supported in this build; log and ignore.
	int32 IndexOut = 0;
	if (GorgeousUIInstanced::TryGetInt(Payload, IndexOut))
	{
		UE_LOG(LogTemp, Warning, TEXT("GorgeousCommonTabList: TabIndex payload received but index->ID resolution is not supported."));
		return;
	}
}

void UGorgeousCommonTabList::OnThemeApplied_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// Tab lists often need specific themed styling for their internal buttons.
	// We can broadcast theme changes to all registered internal children here if needed,
	// but standard CommonUI tab buttons usually register themselves anyway.
}

void UGorgeousCommonTabList::OnThemeApplied_BP_Implementation(const UGorgeousUITheme_DA* Theme)
{
	// Default no-op. Blueprint can override to react to theme changes.
}
