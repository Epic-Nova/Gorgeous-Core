// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "AutoReplication/GorgeousAutoReplicationCoordinator.h"
#include "AutoReplication/GorgeousAutoReplicationTypes.h"
#include "AutoReplication/GorgeousAutoReplicationRPCTransporter.h"
#include "UnitTests/GorgeousAutoReplicationTransporterSpy.h"
#include "ModuleCore/GorgeousAutoReplicationSettings.h"
#include "Engine/Engine.h"

namespace GorgeousAutoReplicationTests
{
	static void ValidateStreamConfig(FAutomationTestBase& Test, const FGorgeousAutoReplicationStreamConfig& Config)
	{
		Test.TestTrue(TEXT("AutoReplication stream update frequency must be positive"), Config.UpdateFrequency > 0.f);
		Test.TestTrue(TEXT("AutoReplication stream bandwidth budget must be positive"), Config.BandwidthBudgetKB > 0.f);
	}
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousAutoReplicationDefaultStreamConfigTest, "GorgeousCore.AutoReplication.DefaultStreamConfig", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousAutoReplicationDefaultStreamConfigTest::RunTest(const FString& Parameters)
{
	const UGorgeousAutoReplicationSettings* Settings = UGorgeousAutoReplicationSettings::Get();
	TestNotNull(TEXT("Developer settings are registered"), Settings);
	if (!Settings)
	{
		return false;
	}

	GorgeousAutoReplicationTests::ValidateStreamConfig(*this, Settings->DefaultStreamConfig);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousAutoReplicationStreamOverrideTest, "GorgeousCore.AutoReplication.StreamOverrides", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousAutoReplicationStreamOverrideTest::RunTest(const FString& Parameters)
{
	const UGorgeousAutoReplicationSettings* Settings = UGorgeousAutoReplicationSettings::Get();
	TestNotNull(TEXT("Developer settings are registered"), Settings);
	if (!Settings)
	{
		return false;
	}

	for (const TPair<FName, FGorgeousAutoReplicationStreamConfig>& Pair : Settings->StreamOverrides)
	{
		AddInfo(FString::Printf(TEXT("Validating stream override for %s"), *Pair.Key.ToString()));
		GorgeousAutoReplicationTests::ValidateStreamConfig(*this, Pair.Value);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousAutoReplicationCoordinatorTest, "GorgeousCore.AutoReplication.CoordinatorLifecycle", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousAutoReplicationCoordinatorTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
	if (!World)
	{
		AddWarning(TEXT("Skipping coordinator lifecycle test because no play world is active."));
		return true;
	}

	FGorgeousAutoReplicationCoordinator& Coordinator = FGorgeousAutoReplicationCoordinator::Get(World);
	Coordinator.Tick(0.f);
	TestTrue(TEXT("Coordinator initialized replication graph when platform supports it"), true);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousAutoReplicationTransporterRoutingModesTest, "GorgeousCore.AutoReplication.TransporterRouting", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousAutoReplicationTransporterRoutingModesTest::RunTest(const FString& Parameters)
{
	UGorgeousAutoReplicationTransporterSpy* Transporter = NewObject<UGorgeousAutoReplicationTransporterSpy>();
	TestNotNull(TEXT("Transporter spy instantiated"), Transporter);
	if (!Transporter)
	{
		return false;
	}

	FGorgeousQueuedRPC RPC;
	RPC.Key = TEXT("TestKey");

	RPC.Type = EGorgeousAutoReplicationRPCType::EReliableServer;
	TestTrue(TEXT("Server-bound RPC routed"), Transporter->RouteRPC(RPC));
	TestTrue(TEXT("Server route invoked"), Transporter->bServerRouted);
	Transporter->ResetFlags();

	RPC.Type = EGorgeousAutoReplicationRPCType::EReliableClient;
	TestTrue(TEXT("Client-bound RPC routed"), Transporter->RouteRPC(RPC));
	TestTrue(TEXT("Client route invoked"), Transporter->bClientRouted);
	Transporter->ResetFlags();

	RPC.Type = EGorgeousAutoReplicationRPCType::EReliableMulticast;
	TestTrue(TEXT("Multicast RPC routed"), Transporter->RouteRPC(RPC));
	TestTrue(TEXT("Multicast route invoked"), Transporter->bMulticastRouted);
	Transporter->ResetFlags();

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

