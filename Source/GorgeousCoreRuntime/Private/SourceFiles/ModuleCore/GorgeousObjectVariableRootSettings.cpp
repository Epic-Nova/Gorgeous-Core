#include "ModuleCore/GorgeousObjectVariableRootSettings.h"

UGorgeousObjectVariableRootSettings::UGorgeousObjectVariableRootSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (RegisteredRoots.Num() == 0)
	{
		// Local standalone root used for non-networked variables.
		{
			FGorgeousObjectVariableRootEntry& DefaultEntry = RegisteredRoots.AddDefaulted_GetRef();
			DefaultEntry.RootName = TEXT("DefaultRoot");
			DefaultEntry.bSupportsNetworking = false;
			DefaultEntry.bEnforceNetworking = false;
			DefaultEntry.bHandlesParentlessReplicatedVariables = false;
			DefaultEntry.bDefault = true;
			DefaultEntry.Description = NSLOCTEXT("GorgeousRootSettings", "DefaultDesc", "Default root for local Gorgeous Object Variables.");
			DefaultConsoleRoot = DefaultEntry.RootName;
		}

		// Opt in root used for variables that may opt into networking features.
		{
			FGorgeousObjectVariableRootEntry& OptInEntry = RegisteredRoots.AddDefaulted_GetRef();
			OptInEntry.RootName = TEXT("DefaultOptInRoot");
			OptInEntry.bSupportsNetworking = true;
			OptInEntry.bEnforceNetworking = false;
			OptInEntry.bHandlesParentlessReplicatedVariables = false;
			OptInEntry.bDefault = false;
			OptInEntry.Description = NSLOCTEXT("GorgeousRootSettings", "DefaultOptInDesc", "Root for Gorgeous Object Variables that may opt into networking features.");
		}

        // Shared root that supports networking but does not enforce it.
        {
            FGorgeousObjectVariableRootEntry& SharedEntry = RegisteredRoots.AddDefaulted_GetRef();
            SharedEntry.RootName = TEXT("DefaultSharedRoot");
            SharedEntry.bSupportsNetworking = true;
            SharedEntry.bEnforceNetworking = false;
			SharedEntry.bHandlesParentlessReplicatedVariables = true;
            SharedEntry.bDefault = false;
            SharedEntry.Description = NSLOCTEXT("GorgeousRootSettings", "DefaultSharedDesc", "Root for Gorgeous Object Variables that may opt into networking features & root reparenting when needed.");
        }

		// Network root that enforces replication + root stack exposure.
		{
			FGorgeousObjectVariableRootEntry& NetworkEntry = RegisteredRoots.AddDefaulted_GetRef();
			NetworkEntry.RootName = TEXT("DefaultNetworkRoot");
			NetworkEntry.bSupportsNetworking = true;
			NetworkEntry.bEnforceNetworking = true;
			NetworkEntry.bHandlesParentlessReplicatedVariables = false;
			NetworkEntry.bDefault = false;
			NetworkEntry.Description = NSLOCTEXT("GorgeousRootSettings", "DefaultNetworkDesc", "Root for Gorgeous Object Variables that must replicate and expose themselves through the network stack.");
		}
	}
}

const UGorgeousObjectVariableRootSettings* UGorgeousObjectVariableRootSettings::Get()
{
	return GetDefault<UGorgeousObjectVariableRootSettings>();
}

const FGorgeousObjectVariableRootEntry* UGorgeousObjectVariableRootSettings::FindRootEntry(const FName RootName) const
{
	if (RootName.IsNone())
	{
		return nullptr;
	}

	return RegisteredRoots.FindByPredicate([&](const FGorgeousObjectVariableRootEntry& Entry)
	{
		return Entry.RootName == RootName;
	});
}

FName UGorgeousObjectVariableRootSettings::GetDefaultRootName() const
{
	if (!DefaultConsoleRoot.IsNone())
	{
		return DefaultConsoleRoot;
	}

	if (const FGorgeousObjectVariableRootEntry* DefaultEntry = RegisteredRoots.FindByPredicate([](const FGorgeousObjectVariableRootEntry& Entry)
	{
		return Entry.bDefault;
	}))
	{
		return DefaultEntry->RootName;
	}

	return RegisteredRoots.Num() > 0 ? RegisteredRoots[0].RootName : NAME_None;
}

TArray<FName> UGorgeousObjectVariableRootSettings::GetRegisteredRootNames() const
{
	TArray<FName> Names;
	Names.Reserve(RegisteredRoots.Num());
	for (const FGorgeousObjectVariableRootEntry& Entry : RegisteredRoots)
	{
		Names.Add(Entry.RootName);
	}
	return Names;
}
