// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousGridPanelProcessor.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonGridPanel.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "Components/GridSlot.h"

UGorgeousGridPanelProcessor::UGorgeousGridPanelProcessor()
{
	TargetWidgetClass = UGorgeousCommonGridPanel::StaticClass();
}

void UGorgeousGridPanelProcessor::OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	UGorgeousCommonGridPanel* GridPanel = Cast<UGorgeousCommonGridPanel>(Widget);
	if (!GridPanel) return;

	// Check if it's the master update payload
	if (const FGorgeousUIUpdatePayload* MasterPayload = Payload.GetPtr<FGorgeousUIUpdatePayload>())
	{
		for (const auto& Pair : MasterPayload->Updates)
		{
			if (const FGorgeousGridUpdatePayload* GridPayload = Pair.Value.GetPtr<FGorgeousGridUpdatePayload>())
			{
				if (GridPayload->bClearChildren)
				{
					GridPanel->ClearChildren();
				}

				if (GridPayload->ChildWidget)
				{
					UGridSlot* NewSlot = GridPanel->AddChildToGrid(GridPayload->ChildWidget, GridPayload->Row, GridPayload->Column);
					//@TODO Additional slot setup could be done here if the payload includes padding/alignment 
				}
			}
		}
	}
	// Or check if it's the direct grid payload
	else if (const FGorgeousGridUpdatePayload* GridPayload = Payload.GetPtr<FGorgeousGridUpdatePayload>())
	{
		if (GridPayload->bClearChildren)
		{
			GridPanel->ClearChildren();
		}

		if (GridPayload->ChildWidget)
		{
			GridPanel->AddChildToGrid(GridPayload->ChildWidget, GridPayload->Row, GridPayload->Column);
		}
	}
}
