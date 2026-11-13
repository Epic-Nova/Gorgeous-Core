// Copyright (c) 2026 Simsalabim Studios. All rights reserved.
#pragma once
#include "CoreMinimal.h"
#include "Features/IModularFeature.h"
class UGorgeousInsightMatrixSubsystem;
class ULocalPlayer;
/** Optional developer presentation service. Core never loads or depends on its implementation. */
class GORGEOUSCORERUNTIME_API IGorgeousDevelopmentToolsService : public IModularFeature
{
public:
    static FName GetFeatureName() { return TEXT("GorgeousDevelopmentToolsService"); }
    virtual ~IGorgeousDevelopmentToolsService() = default;
    virtual bool AllowsShippingTools() const = 0;
    virtual bool IsAvailable() const = 0;
    virtual void Attach(UGorgeousInsightMatrixSubsystem* Subsystem) = 0;
    virtual void Detach() = 0;
    virtual void ShowDebugPanel(int32 ZOrder) = 0;
    virtual void HideDebugPanel() = 0;
    virtual bool IsDebugPanelOpen() const = 0;
    virtual void RefreshDebugPanel() = 0;
    virtual void ShowInGamePanel(int32 ZOrder) = 0;
    virtual void ShowInGamePanelForPlayer(ULocalPlayer* Player, int32 ZOrder) = 0;
    virtual void HideInGamePanel() = 0;
    virtual bool IsInGamePanelOpen() const = 0;
};
