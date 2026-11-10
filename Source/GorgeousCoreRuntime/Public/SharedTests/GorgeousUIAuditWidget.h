// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousCommonWidget.h"
#include "GorgeousUIAuditWidget.generated.h"

// Reflected function fixture exercises the same ProcessEvent path as Blueprint handlers.
UCLASS(NotBlueprintable, Transient)
class UGorgeousUIAuditWidget : public UGorgeousCommonWidget
{
    GENERATED_BODY()
public:
    int32 Handled = 0;
    UFUNCTION() void HandleHUD() { ++Handled; }
};
