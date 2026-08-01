// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Interaction/GorgeousDragDropFoundation.h"

void UGorgeousDragDropFoundation::OnDragStarted_Implementation()
{
}

void UGorgeousDragDropFoundation::OnTargetHighlighted_Implementation(UObject* Target)
{
}

void UGorgeousDragDropFoundation::OnDragConfirmed_Implementation(UObject* Target)
{
}

void UGorgeousDragDropFoundation::ResumeStatefulInteraction(UObject* Target)
{
	OnTargetHighlighted(Target);
	OnDragConfirmed(Target);
}
