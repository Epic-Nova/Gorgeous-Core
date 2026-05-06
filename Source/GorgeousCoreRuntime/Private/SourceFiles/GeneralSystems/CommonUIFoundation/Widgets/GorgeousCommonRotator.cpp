// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonRotator.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperImplementation.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "QualityOfLife/GorgeousPlayerController.h"

UE_UI_IMPLEMENT_WIDGET_INTERFACE(UGorgeousCommonRotator)

void UGorgeousCommonRotator::NativeConstruct()
{
	Super::NativeConstruct();
	UE_UI_REGISTER_WIDGET_USER()

	if (BindingTag.IsValid())
	{
		FSignalBridgeEventDelegate Delegate;
		Delegate.BindDynamic(this, &UGorgeousCommonRotator::OnRotatorSignalReceived);
		USignalBridgeBlueprintFunctionLibrary::Listen(GetWorld(), BindingTag, Cast<AGorgeousPlayerController>(GetOwningPlayer()), Delegate);
	}
}

void UGorgeousCommonRotator::NativeDestruct()
{
	UE_UI_UNREGISTER_WIDGET()
	Super::NativeDestruct();
}

void UGorgeousCommonRotator::OnRotatorSignalReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	if (const FGorgeousInt32Payload_S* IntPayload = Payload.GetPtr<FGorgeousInt32Payload_S>())
	{
		SetSelectedItem(IntPayload->Value);
	}
}

void UGorgeousCommonRotator::ApplyThemeInterpolation(const UGorgeousUITheme_DA* Theme)
{
}
