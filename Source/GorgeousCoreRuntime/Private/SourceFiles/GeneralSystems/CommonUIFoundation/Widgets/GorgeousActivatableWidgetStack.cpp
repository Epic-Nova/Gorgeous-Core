// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousActivatableWidgetStack.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousPrimaryGameLayout.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "StructUtils/InstancedStruct.h"

void UGorgeousActivatableWidgetStack::NativeConstruct()
{
	if (LayerTag.IsValid())
	{
		if (UGorgeousPrimaryGameLayout* RootLayout = UGorgeousPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(this))
		{
			RootLayout->RegisterLayer(LayerTag, this);
		}

		// Also notify via Signal Bridge for dynamic UI registration
		FGameplayTag RegisterTag = FGameplayTag::RequestGameplayTag(FName("UI.System.Layer.Register"));
		FGorgeousRegisterLayerPayload Payload;
		Payload.LayerTag = LayerTag;
		Payload.LayerWidget = this;

		USignalBridgeBlueprintFunctionLibrary::Dispatch(GetWorld(), RegisterTag, FInstancedStruct::Make(Payload));
	}
}