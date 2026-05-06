// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousPanelProcessor.h"
#include "Components/PanelWidget.h"
#include "Components/GridPanel.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "Blueprint/UserWidget.h"

UGorgeousPanelProcessor::UGorgeousPanelProcessor()
{
	// Panel processors usually handle any widget that exposes a 'Panel' property
	TargetWidgetClass = UUserWidget::StaticClass();
}

void UGorgeousPanelProcessor::OnSignalReceived(UObject* Widget, const FInstancedStruct& Payload)
{
	// Panels are not UUserWidgets themselves — we search the widget tree for a panel component
	// Convention: the widget exposes a panel via a property named "Panel" or "Container"
	UPanelWidget* Panel = nullptr;

	if (Widget)
	{
		// Try to get the panel via property reflection first
		if (FObjectProperty* ObjProp = FindFProperty<FObjectProperty>(Widget->GetClass(), FName("Panel")))
		{
			Panel = Cast<UPanelWidget>(ObjProp->GetObjectPropertyValue_InContainer(Widget));
		}
		if (!Panel)
		{
			// Fallback: get the root widget if it's a panel
			Panel = Cast<UPanelWidget>(Cast<UUserWidget>(Widget)->GetRootWidget());
		}
	}

	if (!Panel) return;

	const FGorgeousUIUpdatePayload* MasterPayload = Payload.GetPtr<FGorgeousUIUpdatePayload>();
	if (!MasterPayload) return;

	for (const auto& Pair : MasterPayload->Updates)
	{
		// Check for grid-specific placement payload
		if (const FGorgeousGridUpdatePayload* GridPayload = Pair.Value.GetPtr<FGorgeousGridUpdatePayload>())
		{
			if (UGridPanel* Grid = Cast<UGridPanel>(Panel))
			{
				if (GridPayload->ChildWidget)
				{
					Grid->AddChildToGrid(GridPayload->ChildWidget, GridPayload->Row, GridPayload->Column);
				}
			}
			continue;
		}

		// Check for generic panel update payload
		if (const FGorgeousPanelUpdatePayload* PanelPayload = Pair.Value.GetPtr<FGorgeousPanelUpdatePayload>())
		{
			if (PanelPayload->bClearChildren)
			{
				Panel->ClearChildren();
			}
			if (PanelPayload->WidgetToRemove)
			{
				Panel->RemoveChild(PanelPayload->WidgetToRemove);
			}
			if (PanelPayload->ChildWidget)
			{
				Panel->AddChild(PanelPayload->ChildWidget);
			}
			continue;
		}

		// Fallback to universal reflection on the owning widget
		ApplyPropertyToTarget(Widget, Pair.Key, Pair.Value);
	}
}
