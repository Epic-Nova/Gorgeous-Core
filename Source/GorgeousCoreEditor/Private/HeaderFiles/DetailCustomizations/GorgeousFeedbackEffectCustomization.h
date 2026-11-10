// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once
#include "IDetailCustomization.h"

class FGorgeousFeedbackEffectCustomization final : public IDetailCustomization
{
public:
    static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShared<FGorgeousFeedbackEffectCustomization>(); }
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
