// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonTabList.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperImplementation.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "QualityOfLife/GorgeousPlayerController.h"

UE_UI_IMPLEMENT_WIDGET_INTERFACE(UGorgeousCommonTabList)

void UGorgeousCommonTabList::NativeConstruct()
{
	Super::NativeConstruct();
	UE_UI_REGISTER_WIDGET_USER()

	if (BindingTag.IsValid())
	{
		FSignalBridgeEventDelegate Delegate;
		Delegate.BindDynamic(this, &UGorgeousCommonTabList::OnTabSelectSignalReceived);
		USignalBridgeBlueprintFunctionLibrary::Listen(GetWorld(), BindingTag, Cast<AGorgeousPlayerController>(GetOwningPlayer()), Delegate);
	}
}

void UGorgeousCommonTabList::NativeDestruct()
{
	UE_UI_UNREGISTER_WIDGET()
	Super::NativeDestruct();
}

void UGorgeousCommonTabList::OnTabSelectSignalReceived(FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	if (const FGorgeousInt32Payload_S* IntPayload = Payload.GetPtr<FGorgeousInt32Payload_S>())
	{
		SelectTabByID(GetTabIdAtIndex(IntPayload->Value));
	}
	else if (const FGorgeousFNamePayload_S* NamePayload = Payload.GetPtr<FGorgeousFNamePayload_S>())
	{
		SelectTabByID(NamePayload->Value);
	}
}

void UGorgeousCommonTabList::ApplyThemeInterpolation(const UGorgeousUITheme_DA* Theme)
{
}
