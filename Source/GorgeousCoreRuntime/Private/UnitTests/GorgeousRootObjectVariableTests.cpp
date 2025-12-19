// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "ModuleCore/GorgeousObjectVariableRootSettings.h"

namespace GorgeousRootObjectVariableTests
{
	static UGorgeousObjectVariable* CreateNetworkedVariable(UGorgeousRootObjectVariable* Root, FString&& DisplayLabel, FGuid& OutIdentifier)
	{
		if (!Root)
		{
			return nullptr;
		}

		UGorgeousObjectVariable* Variable = Root->NewObjectVariable(UGorgeousObjectVariable::StaticClass(), OutIdentifier, Root, false, MoveTemp(DisplayLabel));
		if (Variable)
		{
			Variable->SetRootNetworkStackEnabled(true);
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

	UObject* InitialOwner = NewObject<UObject>();
	const FString StableId = TEXT("AutomationClient-001");

	const FGorgeousRootRegistryOwnerHandle InitialHandle = UGorgeousRootObjectVariable::ClaimRootRegistryOwnership(SharedRootName, StableId, InitialOwner);
	TestTrue(TEXT("Ownership handle has a valid token"), InitialHandle.OwnerToken.IsValid());
	TestTrue(TEXT("Ownership handle remembers the stable identifier"), InitialHandle.StableIdentifier == StableId);

	UGorgeousRootObjectVariable::PromoteRootRegistryOwner(SharedRootName, InitialOwner);
	TestEqual(TEXT("Variable uses promoted fallback owner"), Variable->GetAutoReplicationOwner(), InitialOwner);

	UGorgeousRootObjectVariable::ReleaseRootRegistryOwnership(InitialHandle, InitialOwner);
	TestEqual(TEXT("Release keeps fallback owner in place"), Variable->GetAutoReplicationOwner(), InitialOwner);

	UObject* ReturningOwner = NewObject<UObject>();
	FGorgeousRootRegistryOwnerHandle RestoredHandle;
	const bool bRestored = UGorgeousRootObjectVariable::RestoreRootRegistryOwnership(StableId, ReturningOwner, RestoredHandle);
	TestTrue(TEXT("Ownership can be restored via stable identifier"), bRestored);
	TestTrue(TEXT("Restored handle reuses the original token"), bRestored && (RestoredHandle.OwnerToken == InitialHandle.OwnerToken));

	UGorgeousRootObjectVariable::PromoteRootRegistryOwner(RestoredHandle.RootName, ReturningOwner);
	TestEqual(TEXT("Reconnected client becomes the active owner"), Variable->GetAutoReplicationOwner(), ReturningOwner);

	UGorgeousRootObjectVariable::CleanupRegistry();
	TestNotNull(TEXT("Cleanup keeps live variables intact"), UGorgeousRootObjectVariable::FindVariableByIdentifier(VariableIdentifier));

	Root->RemoveVariableFromRegistry(Variable);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
