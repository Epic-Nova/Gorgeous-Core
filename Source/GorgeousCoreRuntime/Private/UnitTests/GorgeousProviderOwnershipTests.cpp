// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "IGorgeousThingsModuleInterface.h"
#include "InsightMatrix/GorgeousInsightMatrixProvider.h"
#include "Features/IModularFeatures.h"

namespace
{
    class FGorgeousAuditOwnedProvider final : public IGorgeousInsightMatrixProvider
    {
    public:
        explicit FGorgeousAuditOwnedProvider(int32& InDestroyed) : Destroyed(InDestroyed) {}
        virtual ~FGorgeousAuditOwnedProvider() override { ++Destroyed; }
        virtual FName GetProviderName() const override { return TEXT("GorgeousAuditProviderOwnership"); }
        virtual FString GetPluginName() const override { return TEXT("GorgeousAuditProviderOwnership"); }
    private:
        int32& Destroyed;
    };

    class FGorgeousAuditProviderModule final : public IGorgeousThingsModuleInterface
    {
    public:
        int32 Destroyed = 0;
        FGorgeousAuditOwnedProvider* Allocated = nullptr;
        IGorgeousInsightMatrixProvider* TypedView() const { return GetInsightProvider(); }
        IModularFeature* FeatureView() const { return InsightProvider; }
        virtual void GorgeousStartupModule() override
        {
            Allocated = new FGorgeousAuditOwnedProvider(Destroyed);
            InsightProvider = static_cast<IModularFeature*>(Allocated);
        }
        virtual void GorgeousShutdownModule() override
        {
            delete static_cast<FGorgeousAuditOwnedProvider*>(InsightProvider);
            InsightProvider = nullptr;
            Allocated = nullptr;
        }
        virtual FName GetBelongingPluginName() const override { return TEXT("GorgeousAuditProviderOwnership"); }
        virtual EGorgeousModuleFunctionality GetModuleFunctionality() const override { return EGorgeousModuleFunctionality::Runtime; }
        virtual TArray<FName> GetDependentPlugins() const override { return {}; }
        virtual int32 GetMinimumRequiredCoreVersion() const override { return 100; }
    };
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousProviderOwnershipRegressionTest,
    "GorgeousCore.RuntimeUtilities.ProviderOwnership",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousProviderOwnershipRegressionTest::RunTest(const FString&)
{
    FGorgeousAuditProviderModule Module;
    Module.CompleteGorgeousStartup();
    TestNotNull(TEXT("Real provider allocation exists"), Module.Allocated);
    TestEqual(TEXT("Typed getter reverses the non-polymorphic base adjustment"),
        Module.TypedView(), static_cast<IGorgeousInsightMatrixProvider*>(Module.Allocated));
    IModularFeature* Feature = Module.FeatureView();
    auto& Features = IModularFeatures::Get();
    const FName Name = IGorgeousInsightMatrixProvider::GetFeatureName();
    bool bRegistered = false;
    for (int32 Index = 0; Index < Features.GetModularFeatureImplementationCount(Name); ++Index)
        bRegistered |= Features.GetModularFeatureImplementation(Name, Index) == Feature;
    TestTrue(TEXT("Actual modular feature is registered through its adjusted base view"), bRegistered);
    static_cast<IModuleInterface&>(Module).ShutdownModule();
    TestEqual(TEXT("Typed ownership invokes the real virtual provider destructor once"), Module.Destroyed, 1);
    TestNull(TEXT("Shutdown clears the non-owning feature view"), Module.FeatureView());
    for (int32 Index = 0; Index < Features.GetModularFeatureImplementationCount(Name); ++Index)
        TestTrue(TEXT("Provider is unregistered before its lifetime ends"), Features.GetModularFeatureImplementation(Name, Index) != Feature);
    static_cast<IModuleInterface&>(Module).ShutdownModule();
    TestEqual(TEXT("Repeated module shutdown cannot destroy a provider twice"), Module.Destroyed, 1);
    return true;
}
#endif
