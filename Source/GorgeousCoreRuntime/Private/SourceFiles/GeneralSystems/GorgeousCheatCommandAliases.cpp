// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#include "GeneralSystems/GorgeousCheatCommandAliases.h"

//<=============================--- Includes ---============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
//<-------------------------------------------------------------------------->

namespace
{
	void ForwardCheatCommand(const TArray<FString>& Args, UWorld* World, const FString& CheatCommand)
	{
		if (!World)
		{
			return;
		}

		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (!PlayerController)
		{
			return;
		}

		const FString Command = Args.IsEmpty()
			? CheatCommand
			: FString::Printf(TEXT("%s %s"), *CheatCommand, *FString::Join(Args, TEXT(" ")));
		PlayerController->ConsoleCommand(Command, true);
	}

	void RegisterAlias(const TCHAR* Name, const TCHAR* Help, const TCHAR* CheatCommand)
	{
		IConsoleManager::Get().RegisterConsoleCommand(
			Name,
			Help,
			FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([CheatCommand](const TArray<FString>& Args, UWorld* World)
			{
				ForwardCheatCommand(Args, World, CheatCommand);
			}),
			ECVF_Default);
	}
}

void FGorgeousCheatCommandAliases::RegisterConsoleCommands()
{
	RegisterAlias(TEXT("gorgeous.stats.set"), TEXT("Sets a statistic: <StatTag> <Value>."), TEXT("Cheat_SetStat"));
	RegisterAlias(TEXT("gorgeous.stats.modify"), TEXT("Modifies a statistic: <StatTag> <Delta>."), TEXT("Cheat_ModifyStat"));
	RegisterAlias(TEXT("gorgeous.stats.list"), TEXT("Lists the local player's statistics."), TEXT("Cheat_ListStats"));

	RegisterAlias(TEXT("gorgeous.permissions.list"), TEXT("Lists registered permission providers."), TEXT("Cheat_ListPermissionProviders"));
	RegisterAlias(TEXT("gorgeous.permissions.override"), TEXT("Sets a provider override: <PermissionName> <true|false>."), TEXT("Cheat_SetPermissionOverride"));
	RegisterAlias(TEXT("gorgeous.permissions.clear-override"), TEXT("Clears a provider override: <PermissionName>."), TEXT("Cheat_ClearPermissionOverride"));
	RegisterAlias(TEXT("gorgeous.permissions.role.create"), TEXT("Creates a role: <RoleName>."), TEXT("Cheat_CreatePermissionRole"));
	RegisterAlias(TEXT("gorgeous.permissions.role.delete"), TEXT("Deletes a role: <RoleName>."), TEXT("Cheat_DeletePermissionRole"));
	RegisterAlias(TEXT("gorgeous.permissions.role.grant-local"), TEXT("Grants the local player a role: <RoleName>."), TEXT("Cheat_GrantLocalPermissionRole"));
	RegisterAlias(TEXT("gorgeous.permissions.role.revoke-local"), TEXT("Revokes a role from the local player: <RoleName>."), TEXT("Cheat_RevokeLocalPermissionRole"));
	RegisterAlias(TEXT("gorgeous.permissions.role.set-rule"), TEXT("Sets a role rule: <RoleName> <PermissionName> <true|false>."), TEXT("Cheat_SetRolePermission"));
	RegisterAlias(TEXT("gorgeous.permissions.evaluate"), TEXT("Evaluates local permission: <PermissionName>."), TEXT("Cheat_EvaluateLocalPermission"));

	RegisterAlias(TEXT("gorgeous.signalbridge.list"), TEXT("Lists Signal Bridge state."), TEXT("Cheat_ListSignalBridge"));
	RegisterAlias(TEXT("gorgeous.signalbridge.register-public"), TEXT("Registers a public signal: <SignalTag>."), TEXT("Cheat_RegisterPublicSignal"));
	RegisterAlias(TEXT("gorgeous.signalbridge.dispatch-local"), TEXT("Dispatches an empty local signal: <SignalTag>."), TEXT("Cheat_DispatchLocalSignal"));
	RegisterAlias(TEXT("gorgeous.signalbridge.clear-listeners"), TEXT("Clears local listeners: <SignalTag>."), TEXT("Cheat_ClearSignalListeners"));

	RegisterAlias(TEXT("gorgeous.interaction.inspect-focus"), TEXT("Inspects local interaction focus."), TEXT("Cheat_InspectInteractionFocus"));
	RegisterAlias(TEXT("gorgeous.interaction.clear-focus"), TEXT("Clears local interaction focus."), TEXT("Cheat_ClearInteractionFocus"));
	RegisterAlias(TEXT("gorgeous.interaction.trace-focus"), TEXT("Traces for interaction focus: <InteractionTag> [Distance]."), TEXT("Cheat_TraceInteractionFocus"));
	RegisterAlias(TEXT("gorgeous.interaction.trace"), TEXT("Traces and executes interaction: <InteractionTag> [Distance]."), TEXT("Cheat_TraceInteraction"));
}
