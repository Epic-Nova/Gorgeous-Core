// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousActivatableWidgetStack.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousPrimaryGameLayout.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
#include "GeneralSystems/SignalBridge/SignalBridgeBlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include GORGEOUS_56_SWITCH("InstancedStruct.h", "StructUtils/InstancedStruct.h")

void UGorgeousActivatableWidgetStack::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();

	if (IsDesignTime())
	{
		return;
	}

	if (LayerTag.IsValid())
	{
		auto* Subsystem = UGorgeousUIFoundationSubsystem::GetForContext(this);
		UGorgeousPrimaryGameLayout* RootLayout = Subsystem ? UGorgeousPrimaryGameLayout::GetPrimaryGameLayout(Subsystem->GetLocalPlayer()) : nullptr;
		
		// If the global lookup fails (common during construction race), check our hierarchy
		if (!RootLayout)
		{
			RootLayout = GetTypedOuter<UGorgeousPrimaryGameLayout>();
		}

		if (RootLayout)
		{
			RootLayout->RegisterLayer(LayerTag, this);
		}

		// Also notify via Signal Bridge for dynamic UI registration
		FGorgeousRegisterLayerPayload Payload;
		Payload.LayerTag = LayerTag;
		Payload.LayerWidget = this;

		if (!RootLayout) USignalBridgeBlueprintFunctionLibrary::DispatchLocal(this, TAG_Gorgeous_UI_Layout_RegisterLayer, FInstancedStruct::Make(Payload));
	}
}
