// Copyright (c) 2026 Simsalabim Studios. All rights reserved.
#pragma once
#include "InsightMatrix/GorgeousDevelopmentToolsService.h"
#include "InsightMatrix/GorgeousDevelopmentSuiteSettings.h"
#include "Features/IModularFeatures.h"
/** A deterministic test service, never registered by production modules. */
class FGorgeousDevelopmentToolsTestScope final : public IGorgeousDevelopmentToolsService
{
public:
    FGorgeousDevelopmentToolsTestScope()
    {
        auto& Features = IModularFeatures::Get();
        bOwnRegistration = !Features.IsModularFeatureAvailable(GetFeatureName());
        if (bOwnRegistration) Features.RegisterModularFeature(GetFeatureName(), this);
        auto* Settings = GetMutableDefault<UGorgeousDevelopmentSuiteSettings>();
        bPrevious = Settings->bEnableInsightMatrix;
        Settings->bEnableInsightMatrix = true;
    }
    ~FGorgeousDevelopmentToolsTestScope()
    {
        GetMutableDefault<UGorgeousDevelopmentSuiteSettings>()->bEnableInsightMatrix = bPrevious;
        if (bOwnRegistration) IModularFeatures::Get().UnregisterModularFeature(GetFeatureName(), this);
    }
    virtual bool AllowsShippingTools() const override { return false; }
    virtual bool IsAvailable() const override { return true; }
    virtual void Attach(UGorgeousInsightMatrixSubsystem*) override {}
    virtual void Detach() override {}
    virtual void ShowDebugPanel(int32) override {}
    virtual void HideDebugPanel() override {}
    virtual bool IsDebugPanelOpen() const override { return false; }
    virtual void RefreshDebugPanel() override {}
    virtual void ShowInGamePanel(int32) override {}
    virtual void ShowInGamePanelForPlayer(ULocalPlayer*, int32) override {}
    virtual void HideInGamePanel() override {}
    virtual bool IsInGamePanelOpen() const override { return false; }
private:
    bool bPrevious = false;
    bool bOwnRegistration = false;
};
