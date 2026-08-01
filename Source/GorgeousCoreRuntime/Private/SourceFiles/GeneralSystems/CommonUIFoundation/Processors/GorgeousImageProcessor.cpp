// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousImageProcessor.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonLazyImage.h"

UGorgeousImageProcessor::UGorgeousImageProcessor()
{
	TargetWidgetClass = UGorgeousCommonLazyImage::StaticClass();
}
