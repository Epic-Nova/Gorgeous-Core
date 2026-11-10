// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once
#include "FeedbackEffects/GorgeousFeedbackEffect.h"
#include "GorgeousFeedbackAuditEffect.generated.h"

UCLASS()
class UGorgeousFeedbackAuditEffect : public UGorgeousFeedbackEffect
{
    GENERATED_BODY()
public:
    int32 Calls = 0;
    TFunction<void()> OnPerformed;
    FTransform ResolveTransformForTest(const FGorgeousFeedbackContext& Context) const { return ResolveEffectTransform(Context); }
protected:
    virtual void PerformExecute(const FGorgeousFeedbackContext&) override { ++Calls; if (OnPerformed) { OnPerformed(); } }
};

UCLASS()
class UGorgeousFeedbackAuditOverrideEffect : public UGorgeousFeedbackAuditEffect
{
    GENERATED_BODY()
public:
    // Models a Blueprint Execute override, which must receive scheduling from the dispatcher.
    virtual void Execute_Implementation(const FGorgeousFeedbackContext&) override { ++Calls; if (OnPerformed) { OnPerformed(); } }
};

UCLASS()
class UGorgeousFeedbackAuditAsyncEffect : public UGorgeousFeedbackAuditEffect
{
    GENERATED_BODY()
public:
    FSimpleDelegate PendingFinish;
    void FinishForTest() { PendingFinish.ExecuteIfBound(); }
protected:
    virtual void PerformExecuteWithCompletion(const FGorgeousFeedbackContext&, FSimpleDelegate OnCompleted) override
    {
        ++Calls;
        PendingFinish = OnCompleted;
        if (OnPerformed) { OnPerformed(); }
    }
};
