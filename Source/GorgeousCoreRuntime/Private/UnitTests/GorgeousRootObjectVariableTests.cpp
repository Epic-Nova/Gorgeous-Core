// *** MARKED_FOR_DELETION ***
// This file's test logic has been migrated to GorgeousRootObjectVariableInsightScenarios.cpp
// as an Insight Matrix scenario.  Delete this file once the migration is confirmed stable.
//
// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*===========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "SharedTests/GorgeousLimitationAuditTestTypes.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ObjectVariables/NativeObjectVariableDefinitions.h"
#include "ModuleCore/GorgeousObjectVariableRootSettings.h"

namespace GorgeousRootObjectVariableTests
{
	static UGorgeousObjectVariable* CreateNetworkedVariable(UGorgeousRootObjectVariable* Root, FString&& DisplayLabel, FGuid& OutIdentifier)
	{
		if (!Root)
		{
			return nullptr;
		}

		UGorgeousObjectVariable* Variable = Root->NewObjectVariable(UInteger_SOV::StaticClass(), OutIdentifier, Root, false, MoveTemp(DisplayLabel));
		if (Variable)
		{
			Variable->SetSharedNetworkStackEnabled(true);
			Variable->SetNetworkingEnabled(true);
		}

		return Variable;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGorgeousRootRegistryOwnershipLifecycleTest, "GorgeousCore.ObjectVariables.RootRegistry.OwnershipLifecycle", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGorgeousRootRegistryOwnershipLifecycleTest::RunTest(const FString& Parameters)
{
	FName SharedRootName = NAME_None;
	if (const UGorgeousObjectVariableRootSettings* RootSettings = UGorgeousObjectVariableRootSettings::Get())
	{
		for (const FGorgeousObjectVariableRootEntry& Entry : RootSettings->RegisteredRoots)
		{
			if (UGorgeousRootObjectVariable::IsSharedNetworkingRoot(Entry.RootName))
			{
				SharedRootName = Entry.RootName;
				break;
			}
		}
	}

	if (SharedRootName.IsNone())
	{
		AddError(TEXT("No shared networking root configured. Configure a shared root (supports networking without enforcement) to run this test."));
		return false;
	}

	UGorgeousRootObjectVariable* Root = UGorgeousRootObjectVariable::GetRootObjectVariable(SharedRootName);
	TestNotNull(TEXT("Default root registry is available"), Root);
	if (!Root)
	{
		return false;
	}

	FGuid VariableIdentifier;
	UGorgeousObjectVariable* Variable = GorgeousRootObjectVariableTests::CreateNetworkedVariable(Root, TEXT("Automation_Var"), VariableIdentifier);
	TestNotNull(TEXT("Network-enabled variable was created"), Variable);
	if (!Variable)
	{
		return false;
	}

    TestTrue(TEXT("Ownership fixture explicitly opts into shared networking"),Variable->bUseSharedNetworkStack);
    TestTrue(TEXT("Shared fixture is eligible for network-owner promotion"),Variable->IsRootNetworkStackEnabled());

	UObject* InitialOwner = NewObject<UGorgeousLimitationAuditReceiver>();
	const FString StableId = TEXT("AutomationClient-001");

	const FGorgeousRootRegistryOwnerHandle InitialHandle = UGorgeousRootObjectVariable::ClaimRootRegistryOwnership(SharedRootName, StableId, InitialOwner);
	TestTrue(TEXT("Ownership handle has a valid token"), InitialHandle.OwnerToken.IsValid());
	TestTrue(TEXT("Ownership handle remembers the stable identifier"), InitialHandle.StableIdentifier == StableId);

	UGorgeousRootObjectVariable::PromoteRootRegistryOwner(SharedRootName, InitialOwner);
	TestEqual(TEXT("Variable uses promoted fallback owner"), Variable->GetAutoReplicationOwner(), InitialOwner);

	UGorgeousRootObjectVariable::ReleaseRootRegistryOwnership(InitialHandle, InitialOwner);
	TestNull(TEXT("Release clears disconnected fallback owner"), Variable->GetAutoReplicationOwner());

	UObject* ReturningOwner = NewObject<UGorgeousLimitationAuditReceiver>();
	FGorgeousRootRegistryOwnerHandle RestoredHandle;
	const bool bRestored = UGorgeousRootObjectVariable::RestoreRootRegistryOwnership(StableId, ReturningOwner, RestoredHandle);
	TestTrue(TEXT("Ownership can be restored via stable identifier"), bRestored);
	TestTrue(TEXT("Restored handle reuses the original token"), bRestored && (RestoredHandle.OwnerToken == InitialHandle.OwnerToken));

	UGorgeousRootObjectVariable::PromoteRootRegistryOwner(RestoredHandle.RootName, ReturningOwner);
	TestEqual(TEXT("Reconnected client becomes the active owner"), Variable->GetAutoReplicationOwner(), ReturningOwner);

	UGorgeousRootObjectVariable::ReleaseRootRegistryOwnership(InitialHandle, InitialOwner);
	TestEqual(TEXT("Stale disconnect cannot release a reconnected owner"), Variable->GetAutoReplicationOwner(), ReturningOwner);
	UGorgeousRootObjectVariable::CleanupRegistry();
	TestNull(TEXT("Level cleanup removes nonpersistent variables"), UGorgeousRootObjectVariable::FindVariableByIdentifier(VariableIdentifier));

	Root->RemoveVariableFromRegistry(Variable);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
