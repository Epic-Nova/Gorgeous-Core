// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|         Gorgeous Core - Comprehensive AutoReplication Test Suite          |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

/**
 * Comprehensive replication test scenarios that exercise EVERY combination of:
 *
 *   1. Transporter Probe, Local routing verification:
 *      - 6 RPC types x 3 target kinds x 2 keys = 36 combos
 *      - Verifies direction, reliability, key, and target kind preservation.
 *
 *   2. Auto-rep PROPERTY replication with CROSS-WORLD VERIFICATION:
 *      - Server => Client, Server => Multicast, Client => Server, Client => Multicast
 *      - Resolves all PIE worlds via GEngine->GetWorldContexts() and categorizes
 *        them as server vs client worlds.
 *      - Injects a random scalar + derived vector on the SENDER world's controller.
 *      - Ticks all net drivers (TickDispatch/TickFlush) in a polling loop until the
 *        RECEIVER world(s)' OV NetReceiveCount increments, or a 2-second timeout.
 *      - Reads back from each RECEIVER controller via TryGetValue (the real
 *        replication consumer path) and proves the exact values match.
 *      - For multicast: ALL receiving worlds are individually verified.
 *      - Falls back to single-world self-read with a warning when only 1 PIE
 *        instance is active (start PIE with 2+ players for genuine cross-world proof).
 *
 *   3. Async Action Return-Value & Per-Responder Tests:
 *      - UGorgeousAutoReplicationRPCRequestAsyncAction full lifecycle test
 *      - OnSingleResponderCompleted per-client callback with progress tracking
 *        (TotalReceivedResponders, TotalExpectedResponders, bIsLastResponder)
 *      - OnCompleted with FGorgeousAutoReplicationRPCAsyncResult
 *      - UGorgeousRPC_OV return-value container validation:
 *        GetCachedTargetVariable(), GetCachedResults(), GetCachedResultMap(),
 *        GetArgumentByName() for round-trip payload verification
 *      - All RPC directions (S->C, C->S, S->MC, C->MC) x Reliable/Unreliable
 *      - All TargetKind variations (Auto, ObjectVariable, Owner)
 *      - Key variations (default key vs alternate key)
 *      - Full 6x3 async-action matrix (18 combos)
 *      - Staggered multi-responder arrival simulation (5 sequential rounds)
 *
 * LOG CAPTURE:
 *   Every scenario captures system warnings and errors during execution via
 *   FGorgeousInsightScopedLogCapture to detect unexpected engine-level issues.
 *   The full log stack is written to result .txt files.
 *
 * LEGITIMATE PIE PROOF:
 *   Every scenario that accesses a UWorld captures a FGorgeousPerfEnvironmentFingerprint
 *   which records NetMode, NetDriver class, connection count, PIE instance ID, player
 *   controller count, map name, frame number, and process ID. This is embedded in the
 *   result notes and metrics, proving the test really ran on a live PIE network endpoint.
 *
 * STATS:
 *   Every scenario returns FGorgeousPerfReplicationStats with timing, counts, value
 *   verification results, and the environment fingerprint. All stats are emitted as
 *   individual metrics in the result.
 */

#include "CoreMinimal.h"
#include "Serialization/MemoryReader.h"
#include "InsightMatrix/GorgeousInsightTestMatrix.h"
#include "InsightMatrix/GorgeousInsightHarness.h"
#include "AutoReplication/GorgeousAutoReplicationTypes.h"
#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "AutoReplication/GorgeousAutoReplicationNetworkingTypes.h"
#include "AutoReplication/GorgeousAutoReplicationRPCRequestAsyncAction.h"
#include "AutoReplication/Globals/GorgeousAutoReplicationRPCPayloadGlobals.h"
#include "AutoReplication/ObjectVariables/GorgeousRPC_OV.h"
#include "ObjectVariables/NativeObjectVariableDefinitions.h"
#include "UnitTests/GorgeousAutoReplicationTransporterProbe.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/NetDriver.h"
#include "HAL/PlatformTime.h"
#include "Containers/Ticker.h"
#include "HAL/PlatformProcess.h"
#include "Math/UnrealMathUtility.h"
#include "SharedTests/GorgeousAsyncActionTestHelper.h"
#include "SharedTests/GorgeousAutoReplicationRPCTestOV.h"
#include "SharedTests/GorgeousObjectVariablePerfTestTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace GorgeousComprehensiveReplicationTests
{
	// ── Constants ───────────────────────────────────────────────────────────

	static const FName DefaultEntryKey(TEXT("GorgeousPlayerController"));
	static const FName AlternateEntryKey(TEXT("GorgeousTestAlternateKey"));

	/** Magic transformation applied to RPC int argument for round-trip verification. */
	static constexpr int32 RPC_TRANSFORM_MULTIPLY = 7;
	static constexpr int32 RPC_TRANSFORM_ADD = 42;

	// ── Proof & Formatting Helpers ─────────────────────────────────────────

	static void AddProof(FGorgeousInsightScenarioResult& Result, bool bCondition, const FString& Message)
	{
		if (!bCondition) { Result.AddError(Message); }
		else { Result.AddNote(FString::Printf(TEXT("PROOF: %s"), *Message)); }
	}

	static FString RoleLabel(bool bIsServer)
	{
		return bIsServer ? TEXT("Server") : TEXT("Client");
	}

	static FString RPCTypeToString(EGorgeousAutoReplicationRPCType Type)
	{
		switch (Type)
		{
		case EGorgeousAutoReplicationRPCType::EReliableServer:      return TEXT("ReliableServer");
		case EGorgeousAutoReplicationRPCType::EReliableClient:      return TEXT("ReliableClient");
		case EGorgeousAutoReplicationRPCType::EReliableMulticast:   return TEXT("ReliableMulticast");
		case EGorgeousAutoReplicationRPCType::EUnreliableServer:    return TEXT("UnreliableServer");
		case EGorgeousAutoReplicationRPCType::EUnreliableClient:    return TEXT("UnreliableClient");
		case EGorgeousAutoReplicationRPCType::EUnreliableMulticast: return TEXT("UnreliableMulticast");
		default: return TEXT("Unknown");
		}
	}

	static FString TargetKindToString(EGorgeousAutoReplicationTargetKind Kind)
	{
		switch (Kind)
		{
		case EGorgeousAutoReplicationTargetKind::EAuto:            return TEXT("Auto");
		case EGorgeousAutoReplicationTargetKind::EObjectVariable:  return TEXT("ObjectVariable");
		case EGorgeousAutoReplicationTargetKind::EOwner:           return TEXT("Owner");
		case EGorgeousAutoReplicationTargetKind::EActorComponent:  return TEXT("ActorComponent");
		default: return TEXT("Unknown");
		}
	}

	static FString GetExpectedDirection(EGorgeousAutoReplicationRPCType Type)
	{
		switch (Type)
		{
		case EGorgeousAutoReplicationRPCType::EReliableServer:
		case EGorgeousAutoReplicationRPCType::EUnreliableServer:
			return TEXT("Server");
		case EGorgeousAutoReplicationRPCType::EReliableClient:
		case EGorgeousAutoReplicationRPCType::EUnreliableClient:
			return TEXT("Client");
		case EGorgeousAutoReplicationRPCType::EReliableMulticast:
		case EGorgeousAutoReplicationRPCType::EUnreliableMulticast:
			return TEXT("Multicast");
		default: return TEXT("Unknown");
		}
	}

	static bool IsReliable(EGorgeousAutoReplicationRPCType Type) 
	{
		return Type == EGorgeousAutoReplicationRPCType::EReliableServer
			|| Type == EGorgeousAutoReplicationRPCType::EReliableClient
			|| Type == EGorgeousAutoReplicationRPCType::EReliableMulticast;
	}

	static UWorld* ResolveWorld(const FGorgeousInsightScenarioContext& Context)
	{
		if (Context.WorldContextObject)
		{
			return Context.WorldContextObject->GetWorld();
		}
		return GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
	}

	static AGorgeousPlayerController* ResolveController(UWorld* World)
	{
		if (!World) return nullptr;
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (AGorgeousPlayerController* PC = Cast<AGorgeousPlayerController>(It->Get()))
			{
				return PC;
			}
		}
		return nullptr;
	}

	/**
	 * Like ResolveController, but returns the FIRST non-local AGorgeousPlayerController.
	 * On a listen server world this is the server-side REPLICA of a connected client's PC —
	 * the exact controller that receives ServerRelayPropertyPayload RPCs for C2S scenarios.
	 * Returns nullptr if no non-local controller is found.
	 */
	static AGorgeousPlayerController* ResolveRemoteController(UWorld* World)
	{
		if (!World) return nullptr;
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (AGorgeousPlayerController* PC = Cast<AGorgeousPlayerController>(It->Get()))
			{
				if (!PC->IsLocalController())
				{
					return PC;
				}
			}
		}
		return nullptr;
	}

	// ── PIE Multi-World Helpers ──────────────────────────────────────────

	/**
	 * Holds all PIE worlds split into server / client buckets.
	 * Populated by ResolvePIEWorlds().
	 */
	struct FPIEWorldSet
	{
		UWorld* ServerWorld = nullptr;
		TArray<UWorld*> ClientWorlds;
		TArray<UWorld*> AllPlayWorlds;

		bool IsValid() const { return ServerWorld != nullptr; }
		bool HasClients() const { return ClientWorlds.Num() > 0; }

		/** Returns all worlds that are NOT the sender. */
		TArray<UWorld*> GetReceiverWorlds(bool bSenderIsServer, bool bMulticast) const
		{
			TArray<UWorld*> Receivers;
			if (bMulticast)
			{
				// Multicast: everyone except the sender receives
				for (UWorld* W : AllPlayWorlds)
				{
					if (bSenderIsServer && W == ServerWorld) continue;
					if (!bSenderIsServer && ClientWorlds.Num() > 0 && W == ClientWorlds[0]) continue;
					Receivers.Add(W);
				}
			}
			else if (bSenderIsServer)
			{
				// Server -> Client: ALL clients are receivers, every connected client
				// must receive and verify the replicated value, not just the first one.
				for (UWorld* W : ClientWorlds)
				{
					Receivers.Add(W);
				}
			}
			else
			{
				// Client -> Server: server receives
				if (ServerWorld)
				{
					Receivers.Add(ServerWorld);
				}
			}
			return Receivers;
		}

		UWorld* GetSenderWorld(bool bSenderIsServer) const
		{
			if (bSenderIsServer) return ServerWorld;
			return ClientWorlds.Num() > 0 ? ClientWorlds[0] : nullptr;
		}
	};

	/**
	 * Iterates GEngine->GetWorldContexts() to find all PIE play worlds.
	 * Categorizes them as server (NM_ListenServer / NM_DedicatedServer)
	 * vs client (NM_Client) based on NetMode.
	 */
	static FPIEWorldSet ResolvePIEWorlds()
	{
		FPIEWorldSet Result;
		if (!GEngine) return Result;

		for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
		{
			UWorld* W = Ctx.World();
			if (!W || Ctx.WorldType != EWorldType::PIE) continue;

			Result.AllPlayWorlds.Add(W);
			const ENetMode NetMode = W->GetNetMode();
			if (NetMode == NM_DedicatedServer || NetMode == NM_ListenServer)
			{
				if (!Result.ServerWorld)
				{
					Result.ServerWorld = W;
				}
			}
			else if (NetMode == NM_Client)
			{
				Result.ClientWorlds.Add(W);
			}
		}
		return Result;
	}

	/**
	 * Ticks all PIE net drivers to flush pending replication, then polls
	 * until the target OV on the receiver has incremented its NetReceiveCount
	 * above the given baseline, or until the timeout expires.
	 *
	 * @param PIEWorlds       The full set of PIE worlds.
	 * @param ReceiverOVs     Array of OV pointers on the receiving side to monitor.
	 * @param BaselineReceiveCounts  Matching baseline NetReceiveCount per OV.
	 * @param bWaitPerReceiver        Per-receiver flag: true = wait for NetReceiveCount to
	 *                                increment (S2C standard property replication fires OnRep),
	 *                                false = relay path only (C2S relay RPC does not fire OnRep
	 *                                on the server receiver, so NetReceiveCount stays at baseline).
	 *                                When all entries are false the function flushes for a short
	 *                                fixed window (0.2s) then returns true immediately.
	 * @param TimeoutSeconds          Max time to poll for true entries (default 2.0s).
	 * @return true if all waiting receivers incremented, or all entries are false (flush-only).
	 */
	static bool TickReplicationUntilReceived(
		const FPIEWorldSet& PIEWorlds,
		const TArray<UGorgeousPerfObjectVariable*>& ReceiverOVs,
		const TArray<int32>& BaselineReceiveCounts,
		const TArray<bool>& bWaitPerReceiver,
		double TimeoutSeconds = 2.0)
	{
		const double StartTime = FPlatformTime::Seconds();
		constexpr double PollIntervalSeconds = 0.010; // 10ms per poll

		const bool bAnyWaiting = bWaitPerReceiver.ContainsByPredicate([](bool b){ return b; });
		const double EffectiveTimeout = bAnyWaiting ? TimeoutSeconds : 0.2;

		while (FPlatformTime::Seconds() - StartTime < EffectiveTimeout)
		{
			// Tick all net drivers to push replication through
			for (UWorld* W : PIEWorlds.AllPlayWorlds)
			{
				if (UNetDriver* ND = W->GetNetDriver())
				{
					// On the server side, this drives ServerReplicateActors
					ND->TickDispatch(PollIntervalSeconds);
					ND->TickFlush(PollIntervalSeconds);
				}
			}

			// Drive the core ticker so that FTSTicker-based delegates fire, this includes
			// the server-side HandleServerPropertyPollingTick and client-side
			// HandleClientPropertyPollingTick registered via StartServer/ClientPropertyPolling.
			// Without this, the mixin polling tickers are never advanced during the wait loop
			// because FTSTicker only runs during the normal engine Tick, which is blocked.
			FTSTicker::GetCoreTicker().Tick(static_cast<float>(PollIntervalSeconds));

			if (bAnyWaiting)
			{
				// Return early once every waiting receiver has seen its count increment.
				bool bAllReceived = true;
				for (int32 i = 0; i < ReceiverOVs.Num(); ++i)
				{
					if (!bWaitPerReceiver.IsValidIndex(i) || !bWaitPerReceiver[i]) continue;
					if (ReceiverOVs[i] && ReceiverOVs[i]->GetPerfNetReceiveCount() <= BaselineReceiveCounts[i])
					{
						bAllReceived = false;
						break;
					}
				}
				if (bAllReceived) return true;
			}

			FPlatformProcess::Sleep(static_cast<float>(PollIntervalSeconds));
		}

		// flush-only path always succeeds; waiting path timed out
		return !bAnyWaiting;
	}

	/**
	 * Sets up a UGorgeousPerfObjectVariable on the given controller with
	 * the standard networking config + registers it in the mixin.
	 * Returns the created OV or nullptr on failure.
	 */
	static UGorgeousPerfObjectVariable* SetupReplicatedOV(
		AGorgeousPlayerController* Controller,
		FName EntryKey)
	{
		UGorgeousPerfObjectVariable* Variable = NewObject<UGorgeousPerfObjectVariable>(Controller);
		if (!Variable) return nullptr;

		Variable->bSupportsNetworking = true;
		Variable->ReplicationMode = EGorgeousObjectVariableReplicationMode::EFullAutoReplication;
		Variable->RootNetworkConfig.bExposeThroughRootNetworkStack = true;
		Variable->RootNetworkConfig.AccessPolicy = EGorgeousObjectVariableAccessPolicy::Everyone;
		Variable->RootNetworkConfig.ReplicationChannel = EntryKey;
		Variable->bUseSharedNetworkStack = true;
		Variable->EnsureSharedNetworkStackOwner(Controller);

		FGorgeousObjectVariableEntry& Entry = Controller->AdditionalGorgeousData.FindOrAdd(EntryKey);
		Entry.DefaultValue = Variable;
		Entry.bReplicate = true;
		Entry.Handle.CacheValue(Variable);

		// Allocate a replication slot in ReplicatedVariables for this entry.
		// InitializeAdditionalData iterates AdditionalGorgeousData, sees bReplicate=true,
		// and calls GetOrAssignReplicationIndex + populates the replicated array.
		// Without this, TrySetReplicatedValue will fail with "no replication slot registered".
		// The call chain is: InitializeAdditionalData -> Handle.CacheValue -> SetAutoReplicationBinding
		//   -> ActivateReplication (registers base properties + calls OnReplicationActivated_Implementation
		//      which registers PerfReplicationPayload on UGorgeousPerfBaseObjectVariable).
		Controller->GetAutoReplicationMixin().InitializeAdditionalData(true);
		Controller->GetAutoReplicationMixin().TrySetReplicatedValue(EntryKey, Variable);

		return Variable;
	}

	/**
	 * Registers a UGorgeousAutomationTestOV under EntryKey on the controller so that
	 * ResolveVariableForKey() finds a candidate for the EAuto / EObjectVariable RPC paths.
	 * The test subclass has Automation_HandleRPC_WithReturnOV declared as a UFUNCTION so
	 * that InvokeNativeHandler can locate it by name during dispatch.
	 * Idempotent: returns the existing entry's variable if one is already cached.
	 * Does NOT set up property-replication slots, this OV is solely for RPC dispatch.
	 */
	static UGorgeousObjectVariable* SetupAsyncRPCTestOV(
		AGorgeousPlayerController* Controller,
		FName EntryKey)
	{
		if (!Controller) return nullptr;

		// Idempotent only when the cached value is already a UGorgeousAutomationTestOV.
		// If a different OV subclass is cached (e.g. GorgeousPerfObjectVariable registered by
		// the property-rep test), we must replace the Handle cache with a TestOV so that
		// InvokeNativeHandler can find Automation_HandleRPC_WithReturnOV by name.
		// DefaultValue is intentionally left untouched, property-rep tests may still need it.
		if (FGorgeousObjectVariableEntry* Existing = Controller->AdditionalGorgeousData.Find(EntryKey))
		{
			if (UGorgeousAutomationTestOV* Cached = Cast<UGorgeousAutomationTestOV>(Existing->Handle.GetCachedValue()))
			{
				return Cached;  // Truly idempotent, already the right type.
			}
			// Wrong OV type in cache, create a TestOV and overwrite just the handle.
			UGorgeousAutomationTestOV* TestOV = NewObject<UGorgeousAutomationTestOV>(Controller);
			if (!TestOV) return nullptr;
			TestOV->bSupportsNetworking = true;
			Existing->Handle.CacheValue(TestOV);
			// Do NOT call InitializeAdditionalData here, the entry already exists so the
			// mixin is already initialised. Calling it would rebuild AdditionalData and erase
			// the CacheValue we just wrote above.
			return TestOV;
		}

		// Use the test-specific subclass so InvokeNativeHandler can find Automation_HandleRPC_WithReturnOV
		// on the OV instance for EObjectVariable / EAuto dispatch paths.
		UGorgeousAutomationTestOV* Variable = NewObject<UGorgeousAutomationTestOV>(Controller);
		if (!Variable) return nullptr;

		// Minimal networking config, just enough for ExecuteAutoReplicationRPC to accept the call.
		Variable->bSupportsNetworking = true;

		FGorgeousObjectVariableEntry& Entry = Controller->AdditionalGorgeousData.FindOrAdd(EntryKey);
		Entry.DefaultValue = Variable;
		Entry.bReplicate  = false;  // RPC-test OV does not need property-replication slots.
		Entry.Handle.CacheValue(Variable);

		// Enable networking on the mixin so RequestRPC passes the IsNetworkingEnabled() guard.
		Controller->GetAutoReplicationMixin().InitializeAdditionalData(true);

		return Variable;
	}

	/** Generate a random string of specified length for verification. */
	static FString GenerateRandomString(FRandomStream& Stream, int32 Length = 16)
	{
		static const TCHAR Alphabet[] = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
		const int32 AlphabetLen = UE_ARRAY_COUNT(Alphabet) - 1;
		FString Result;
		Result.Reserve(Length);
		for (int32 i = 0; i < Length; ++i)
		{
			Result.AppendChar(Alphabet[Stream.RandRange(0, AlphabetLen - 1)]);
		}
		return Result;
	}

	/** Embed PIE environment proof into the result. */
	static FGorgeousPerfEnvironmentFingerprint EmbedEnvironmentProof(
		UWorld* World,
		FGorgeousInsightScenarioResult& Result,
		const TCHAR* Label)
	{
		FGorgeousPerfEnvironmentFingerprint FP = FGorgeousPerfEnvironmentFingerprint::Capture(World);

		Result.AddNote(FString::Printf(TEXT("PIE-PROOF [%s]: %s"), Label, *FP.ToString()));

		if (FP.IsRealNetEnvironment())
		{
			AddProof(Result, true,
				FString::Printf(TEXT("[%s] Running on REAL PIE net environment: NetMode=%s, NetDriver=%s, Connections=%d, PIEInstance=%d"),
					Label, *FP.NetModeName, *FP.NetDriverName, FP.NumServerConnections, FP.PIEInstanceId));
		}
		else
		{
			Result.AddWarning(FString::Printf(
				TEXT("[%s] NOT a real PIE net environment (PIE=%s, NetDriver=%s, NetMode=%s). "
				     "Start PIE with multiplayer to get legitimate replication proof."),
				Label,
				FP.bIsPlayInEditor ? TEXT("Yes") : TEXT("No"),
				FP.bHasNetDriver ? TEXT("Yes") : TEXT("No"),
				*FP.NetModeName));
		}

		FP.EmitMetrics(Result, FString::Printf(TEXT("%s.env"), Label));
		return FP;
	}

	/**
	 * Build an RPC payload that carries random test values for round-trip verification.
	 * Adds: "TestInputInt" (random int), "TestInputString" (random string),
	 *       "Origin" (role label), "Timestamp" (ISO timestamp), "Sequence" (variant).
	 */
	static FGorgeousRPCPayload BuildVerifiablePayload(
		UObject* Context,
		const TCHAR* HandlerName,
		int32 SequenceId,
		bool bIsServer,
		int32 RandomInt,
		const FString& RandomString)
	{
		FGorgeousRPCPayload Payload = UGorgeousAutoReplicationRPCPayloadGlobals::MakeAutoReplicationRPCPayload(HandlerName);
		UGorgeousAutoReplicationRPCPayloadGlobals::AddAutoReplicationRPCIntArgument(Payload, TEXT("TestInputInt"), RandomInt);
		UGorgeousAutoReplicationRPCPayloadGlobals::AddAutoReplicationRPCStringArgument(Payload, TEXT("TestInputString"), RandomString);
		UGorgeousAutoReplicationRPCPayloadGlobals::AddAutoReplicationRPCIntArgument(Payload, TEXT("Sequence"), SequenceId);
		UGorgeousAutoReplicationRPCPayloadGlobals::AddAutoReplicationRPCStringArgument(Payload, TEXT("Origin"), *RoleLabel(bIsServer));
		UGorgeousAutoReplicationRPCPayloadGlobals::AddAutoReplicationRPCStringArgument(Payload, TEXT("Timestamp"),
			FDateTime::UtcNow().ToString(TEXT("yyyy-MM-dd_HH-mm-ss-fff")));
		return Payload;
	}

	// =====================================================================
	// SECTION 1:  Transporter Probe -- Local routing verification with
	//             full direction + reliability + target kind proof.
	// =====================================================================

	static FGorgeousInsightScenarioResult RunTransporterProbeMatrix(const FGorgeousInsightScenarioContext& Context)
	{
		FGorgeousInsightScenarioResult Result;
		FGorgeousInsightScopedLogCapture LogCapture;
		const double StartTime = FPlatformTime::Seconds();

		UGorgeousAutoReplicationTransporterProbe* Probe = NewObject<UGorgeousAutoReplicationTransporterProbe>();
		AddProof(Result, Probe != nullptr, TEXT("Transporter probe instantiated"));
		if (!Probe) return Result;

		const TArray<EGorgeousAutoReplicationRPCType> AllRPCTypes = {
			EGorgeousAutoReplicationRPCType::EReliableServer,
			EGorgeousAutoReplicationRPCType::EReliableClient,
			EGorgeousAutoReplicationRPCType::EReliableMulticast,
			EGorgeousAutoReplicationRPCType::EUnreliableServer,
			EGorgeousAutoReplicationRPCType::EUnreliableClient,
			EGorgeousAutoReplicationRPCType::EUnreliableMulticast
		};

		const TArray<EGorgeousAutoReplicationTargetKind> AllTargetKinds = {
			EGorgeousAutoReplicationTargetKind::EAuto,
			EGorgeousAutoReplicationTargetKind::EObjectVariable,
			EGorgeousAutoReplicationTargetKind::EOwner
		};

		const TArray<FName> AllKeys = { DefaultEntryKey, AlternateEntryKey };

		int32 TotalCombinations = 0;
		int32 PassedCombinations = 0;

		for (EGorgeousAutoReplicationRPCType RPCType : AllRPCTypes)
		{
			for (EGorgeousAutoReplicationTargetKind Kind : AllTargetKinds)
			{
				for (const FName& Key : AllKeys)
				{
					++TotalCombinations;
					Probe->ResetProbe();

					FGorgeousQueuedRPC RPC;
					RPC.Key = Key;
					RPC.Type = RPCType;
					RPC.TargetKind = Kind;
					RPC.Payload.HandlerName = FName(*FString::Printf(TEXT("Handler_%s_%s_%s"),
						*RPCTypeToString(RPCType), *TargetKindToString(Kind), *Key.ToString()));

					const bool bRouted = Probe->RouteRPC(RPC);
					const FString ExpectedDir = GetExpectedDirection(RPCType);
					const bool bReliable = IsReliable(RPCType);
					const bool bCorrectDirection = Probe->HasRouteIn(ExpectedDir);

					const FString CombinationLabel = FString::Printf(TEXT("[%s | %s | Key=%s]"),
						*RPCTypeToString(RPCType), *TargetKindToString(Kind), *Key.ToString());

					if (bRouted && bCorrectDirection)
					{
						++PassedCombinations;
						const TArray<FGorgeousTransporterProbeEntry> Entries = Probe->GetEntriesForDirection(ExpectedDir);
						if (Entries.Num() > 0)
						{
							const FGorgeousTransporterProbeEntry& E = Entries.Last();
							AddProof(Result, E.bReliable == bReliable,
								FString::Printf(TEXT("%s reliability=%s (expected %s)"),
									*CombinationLabel,
									E.bReliable ? TEXT("reliable") : TEXT("unreliable"),
									bReliable ? TEXT("reliable") : TEXT("unreliable")));

							AddProof(Result, E.TargetKind == Kind,
								FString::Printf(TEXT("%s TargetKind=%s preserved through routing"), *CombinationLabel, *TargetKindToString(Kind)));

							AddProof(Result, E.Key == Key,
								FString::Printf(TEXT("%s Key=%s preserved through routing"), *CombinationLabel, *Key.ToString()));

							Result.AddNote(FString::Printf(TEXT("PROOF: %s Started on %s -> Routed to %s (%s)"),
								*CombinationLabel, *E.CallerLabel, *ExpectedDir,
								E.bReliable ? TEXT("reliable") : TEXT("unreliable")));
						}
					}
					else
					{
						Result.AddError(FString::Printf(TEXT("%s FAILED: routed=%s, correctDirection=%s"),
							*CombinationLabel, bRouted ? TEXT("true") : TEXT("false"), bCorrectDirection ? TEXT("true") : TEXT("false")));
					}
				}
			}
		}

		const double EndTime = FPlatformTime::Seconds();
		Result.AddMetric(TEXT("probe.totalCombinations"), static_cast<double>(TotalCombinations));
		Result.AddMetric(TEXT("probe.passedCombinations"), static_cast<double>(PassedCombinations));
		Result.AddMetric(TEXT("probe.durationS"), EndTime - StartTime, TEXT("s"));
		Result.AddMetric(TEXT("probe.avgPerComboS"), (EndTime - StartTime) / FMath::Max(1, TotalCombinations), TEXT("s"));
		Result.AddNote(FString::Printf(TEXT("Transporter probe matrix: %d/%d combinations passed in %.4fs"), PassedCombinations, TotalCombinations, EndTime - StartTime));
		LogCapture.ApplyToResult(Result);
		Result.LogCapture = MoveTemp(LogCapture.FullLogLines);
		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakeTransporterProbeMatrix()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("AutoReplication.Comprehensive.TransporterProbeMatrix");
		D.DisplayName  = TEXT("Transporter Probe: Full Input Matrix (36 combos)");
		D.Description  = TEXT("Routes every combination of 6 RPC types x 3 target kinds x 2 keys through the probe and verifies direction, reliability, key, and target kind preservation.");
		D.Tags         = { TEXT("local"), TEXT("transporter"), TEXT("rpc"), TEXT("comprehensive"), TEXT("matrix") };
		D.Priority     = 150;
		D.Runner       = &RunTransporterProbeMatrix;
		return D;
	}

	// =====================================================================
	// SECTION 2:  Property Replication Direction Tests
	//             Server => Client, Server => Multicast,
	//             Client => Server, Client => Multicast
	//
	//   Each test generates a RANDOM SCALAR on the SENDER world,
	//   injects it into the OV, then TICKS replication until the
	//   RECEIVER world(s) have received the data via their net
	//   driver. Readback is performed on the RECEIVER controller(s)
	//   via TryGetValue, the actual replication consumer path.
	//   For multicast, ALL receiving worlds are individually verified.
	//   The PIE environment fingerprint is embedded as proof.
	// =====================================================================

	static FGorgeousInsightScenarioResult RunPropertyReplication(
		const FGorgeousInsightScenarioContext& Context,
		const TCHAR* Label,
		bool bExpectServerOrigin,
		bool bExpectMulticastTarget)
	{
		FGorgeousInsightScenarioResult Result;
		FGorgeousInsightScopedLogCapture LogCapture;
		FGorgeousPerfReplicationStats Stats;
		Stats.OperationStartSeconds = FPlatformTime::Seconds();

		// -- Resolve all PIE worlds (server + clients) --
		FPIEWorldSet PIEWorlds = ResolvePIEWorlds();

		// Fallback: if no PIE worlds found, try the single-world path
		if (!PIEWorlds.IsValid())
		{
			UWorld* FallbackWorld = ResolveWorld(Context);
			if (FallbackWorld)
			{
				PIEWorlds.AllPlayWorlds.Add(FallbackWorld);
				const ENetMode NetMode = FallbackWorld->GetNetMode();
				if (NetMode == NM_DedicatedServer || NetMode == NM_ListenServer)
				{
					PIEWorlds.ServerWorld = FallbackWorld;
				}
				else
				{
					// Standalone: treat as server for the write side
					PIEWorlds.ServerWorld = FallbackWorld;
				}
			}
		}

		if (!PIEWorlds.IsValid())
		{
			Result.AddWarning(TEXT("No play world active -- this scenario requires PIE or Gauntlet."));
			LogCapture.ApplyToResult(Result);
			Result.LogCapture = MoveTemp(LogCapture.FullLogLines);
			return Result;
		}

		// -- Determine sender / receiver worlds based on intended direction --
		UWorld* SenderWorld = PIEWorlds.GetSenderWorld(bExpectServerOrigin);
		TArray<UWorld*> ReceiverWorlds = PIEWorlds.GetReceiverWorlds(bExpectServerOrigin, bExpectMulticastTarget);

		if (!SenderWorld)
		{
			Result.AddError(FString::Printf(TEXT("[%s] Cannot resolve sender world (senderIsServer=%s)"),
				Label, bExpectServerOrigin ? TEXT("true") : TEXT("false")));
			LogCapture.ApplyToResult(Result);
			Result.LogCapture = MoveTemp(LogCapture.FullLogLines);
			return Result;
		}

		// If there are no separate receiver worlds (single PIE, no multiplayer),
		// fall back to verifying on the sender's own world (same as before, with
		// a warning that cross-world verification is not possible).
		const bool bCrossWorldVerification = ReceiverWorlds.Num() > 0;
		if (!bCrossWorldVerification)
		{
			ReceiverWorlds.Add(SenderWorld);
			Result.AddWarning(FString::Printf(
				TEXT("[%s] No separate receiver world available, verifying on sender's own world. "
				     "Start PIE with 2+ players to get genuine cross-world replication proof."),
				Label));
		}

		// -- Capture PIE environment proof from sender --
		Stats.Fingerprint = EmbedEnvironmentProof(SenderWorld, Result, Label);

		// -- Resolve sender controller --
		AGorgeousPlayerController* SenderController = ResolveController(SenderWorld);
		if (!SenderController)
		{
			Result.AddError(FString::Printf(TEXT("[%s] No GorgeousPlayerController found on sender world."), Label));
			LogCapture.ApplyToResult(Result);
			Result.LogCapture = MoveTemp(LogCapture.FullLogLines);
			return Result;
		}

		const bool bSenderIsServer = SenderController->HasAuthority();
		const FString Origin = RoleLabel(bSenderIsServer);
		const FString Target = bExpectMulticastTarget ? TEXT("Multicast (All)") : (bExpectServerOrigin ? TEXT("Client") : TEXT("Server"));

		// Verify the test actually started on the correct side. In a genuine multi-player
		// PIE session this always holds, but in the single-world standalone fallback both
		// S2C and C2S can land on the same world, surface that as a hard failure rather
		// than silently producing a misleading cross-direction result.
		const bool bCorrectSide = (bSenderIsServer == bExpectServerOrigin);
		AddProof(Result, bCorrectSide,
			FString::Printf(TEXT("[%s] Test started on the correct side: expected %s, got %s"),
				Label,
				bExpectServerOrigin ? TEXT("Server") : TEXT("Client"),
				bSenderIsServer     ? TEXT("Server") : TEXT("Client")));

		if (!bCorrectSide)
		{
			Result.AddError(FString::Printf(
				TEXT("[%s] Wrong-side start: expected %s world but controller HasAuthority()=%s. "
				     "Ensure PIE is running with multiplayer (2+ players) so server and client worlds are distinct."),
				Label,
				bExpectServerOrigin ? TEXT("Server") : TEXT("Client"),
				bSenderIsServer     ? TEXT("true")   : TEXT("false")));
			LogCapture.ApplyToResult(Result);
			Result.LogCapture = MoveTemp(LogCapture.FullLogLines);
			return Result;
		}

		Result.AddNote(FString::Printf(TEXT("PROOF: [%s] PIE topology: %d total worlds, 1 server, %d clients. CrossWorld=%s"),
			Label, PIEWorlds.AllPlayWorlds.Num(), PIEWorlds.ClientWorlds.Num(),
			bCrossWorldVerification ? TEXT("YES") : TEXT("NO")));

		// -- Set up OV on the sender --
		UGorgeousPerfObjectVariable* SenderOV = SetupReplicatedOV(SenderController, DefaultEntryKey);
		AddProof(Result, SenderOV != nullptr, FString::Printf(TEXT("[%s] Sender OV created on %s"), Label, *Origin));
		if (!SenderOV)
		{
			LogCapture.ApplyToResult(Result);
			Result.LogCapture = MoveTemp(LogCapture.FullLogLines);
			return Result;
		}

		// -- Also set up OVs on ALL receiver controllers so TryGetValue has an entry --
		TArray<AGorgeousPlayerController*> ReceiverControllers;
		TArray<UGorgeousPerfObjectVariable*> ReceiverOVs;
		TArray<int32> BaselineReceiveCounts;
		// Per-receiver: true  = Standard UE property replication (OnRep fires, NetReceiveCount increments), S2C / C2MC second client
		//               false = Relay RPC path (OnRep never fires on the receiver)          , C2S server / C2MC server
		TArray<bool> bExpectNetReceiveCountPerReceiver;

		for (int32 i = 0; i < ReceiverWorlds.Num(); ++i)
		{
			UWorld* RW = ReceiverWorlds[i];

			// For C2S / C2MC scenarios the sender is a client and the relay RPC
			// (ServerRelayPropertyPayload) arrives on the SERVER's REPLICA of the
			// client's PlayerController, a non-local controller on the server world.
			// Using ResolveController (which returns the first/local PC) would set up
			// the wrong controller, causing "UnknownEntry" discards on the relay path.
			const bool bReceiverIsServerWorld = (RW == PIEWorlds.ServerWorld);
			const bool bNeedRemoteController  = !bExpectServerOrigin && bReceiverIsServerWorld;
			AGorgeousPlayerController* RecvPC = bNeedRemoteController
				? ResolveRemoteController(RW)
				: ResolveController(RW);

			// Fallback: if no remote controller found (e.g. single-world PIE), try local
			if (!RecvPC && bNeedRemoteController)
			{
				Result.AddWarning(FString::Printf(
					TEXT("[%s] No non-local controller on server world %d, falling back to local. "
					     "C2S relay may not be delivered to the expected controller."), Label, i));
				RecvPC = ResolveController(RW);
			}

			if (!RecvPC)
			{
				Result.AddWarning(FString::Printf(TEXT("[%s] No controller on receiver world %d"), Label, i));
				continue;
			}

			// If this is the same controller as the sender (single-world fallback),
			// don't create a second OV, use the sender's OV.
			if (RecvPC == SenderController)
			{
				ReceiverControllers.Add(RecvPC);
				ReceiverOVs.Add(SenderOV);
				BaselineReceiveCounts.Add(SenderOV->GetPerfNetReceiveCount());
				bExpectNetReceiveCountPerReceiver.Add(false); // single-world, no cross-world count
				continue;
			}

			// Cross-world: set up an equivalent OV on the receiver so the mixin
			// has entry + replication slot for the key. Replication will overwrite
			// the receiver's OV payload with the sender's data.
			UGorgeousPerfObjectVariable* RecvOV = SetupReplicatedOV(RecvPC, DefaultEntryKey);
			if (!RecvOV)
			{
				Result.AddWarning(FString::Printf(TEXT("[%s] Failed to set up receiver OV on world %d"), Label, i));
				continue;
			}

			ReceiverControllers.Add(RecvPC);
			ReceiverOVs.Add(RecvOV);
			BaselineReceiveCounts.Add(RecvOV->GetPerfNetReceiveCount());
			// Relay receiver (server in C2S/C2MC): OnRep never fires → don't wait for count.
			// Standard S2C receiver (client): OnRep fires → wait for count.
			bExpectNetReceiveCountPerReceiver.Add(!bNeedRemoteController);
		}

		if (ReceiverControllers.Num() == 0)
		{
			Result.AddError(FString::Printf(TEXT("[%s] No receiver controllers could be resolved"), Label));
			LogCapture.ApplyToResult(Result);
			Result.LogCapture = MoveTemp(LogCapture.FullLogLines);
			return Result;
		}

		// -- Generate random value --
		Stats.RandomSeed = FMath::RandRange(1, 999999);
		FRandomStream RandStream(Stats.RandomSeed);
		const float RandomScalar = RandStream.GetFraction();
		const float StampSeconds = static_cast<float>(FPlatformTime::Seconds());

		Stats.InjectedScalar = RandomScalar;

		// Compute expected derived vector (mirrors InjectReplicationStimulus logic)
		const int32 PreInjectionSeq = SenderOV->GetPerfReplicationStimulusCount();
		const int32 ExpectedSeq = PreInjectionSeq + 1;
		const float Spread = FMath::Fmod(RandomScalar * 811.0f, 512.0f);
		Stats.InjectedVector = FVector(RandomScalar * 1024.0f, static_cast<float>(ExpectedSeq), Spread + StampSeconds * 0.25f);
		Stats.InjectedSequence = ExpectedSeq;

		// -- Inject stimulus on SENDER --
		SenderOV->InjectReplicationStimulus(StampSeconds, RandomScalar);
		SenderController->ForceNetUpdate();

		Result.AddNote(FString::Printf(TEXT("PROOF: [%s] Stimulus injected on %s (seed=%d, scalar=%.6f)"),
			Label, *Origin, Stats.RandomSeed, Stats.InjectedScalar));

		// -- Tick replication and wait for receivers --
		if (bCrossWorldVerification)
		{
			const double ReplicationWaitStart = FPlatformTime::Seconds();
			// For S2C / S2MC: standard UE property replication fires OnRep on the client,
			// incrementing NetReceiveCount, wait for that. For C2S / C2MC (relay path):
			// OnRep never fires on the server receiver, so just flush net drivers briefly
			// and verify by comparing actual values instead.
			const bool bReplicationArrived = TickReplicationUntilReceived(
				PIEWorlds, ReceiverOVs, BaselineReceiveCounts, bExpectNetReceiveCountPerReceiver);
			const double ReplicationWaitDuration = FPlatformTime::Seconds() - ReplicationWaitStart;

			Result.AddMetric(FString::Printf(TEXT("%s.replicationWaitS"), Label), ReplicationWaitDuration, TEXT("s"));
			AddProof(Result, bReplicationArrived,
				FString::Printf(TEXT("[%s] Replication arrived at %d/%d receivers in %.4fs"),
					Label, bReplicationArrived ? ReceiverOVs.Num() : 0, ReceiverOVs.Num(), ReplicationWaitDuration));

			if (!bReplicationArrived)
			{
				Result.AddWarning(FString::Printf(
					TEXT("[%s] Replication timed out after 2.0s, receiver OVs may not have updated. "
					     "Readback values will be checked but may not match."),
					Label));
			}
		}

		// -- Read back from EACH receiver via TryGetValue --
		int32 ReceiversVerified = 0;
		for (int32 i = 0; i < ReceiverControllers.Num(); ++i)
		{
			AGorgeousPlayerController* RecvPC = ReceiverControllers[i];
			UGorgeousPerfObjectVariable* RecvOV = ReceiverOVs[i];
			const FString RecvLabel = FString::Printf(TEXT("%s.Recv%d"), Label, i);
			const bool bIsSenderWorld = (RecvPC == SenderController);

			// Read through TryGetValue, the real replication consumer path
			UGorgeousObjectVariable* ReadBackOV = nullptr;
			const bool bReadBackFound = RecvPC->GetAutoReplicationMixin().TryGetValue(DefaultEntryKey, ReadBackOV);
			AddProof(Result, bReadBackFound && ReadBackOV != nullptr,
				FString::Printf(TEXT("[%s] TryGetValue('%s') on %s returned %s"),
					*RecvLabel, *DefaultEntryKey.ToString(),
					bIsSenderWorld ? TEXT("sender(fallback)") : TEXT("receiver"),
					bReadBackFound ? TEXT("FOUND") : TEXT("NOT FOUND")));

			if (!bReadBackFound || !ReadBackOV) continue;

			UGorgeousPerfObjectVariable* ReadBackPerfOV = Cast<UGorgeousPerfObjectVariable>(ReadBackOV);
			AddProof(Result, ReadBackPerfOV != nullptr,
				FString::Printf(TEXT("[%s] ReadBack OV cast to UGorgeousPerfObjectVariable = %s"),
					*RecvLabel, ReadBackPerfOV ? TEXT("OK") : TEXT("FAILED")));

			if (!ReadBackPerfOV) continue;

			const FGorgeousPerfReplicationStimulus& Payload = ReadBackPerfOV->GetPerfReplicationPayload();

			const float RBScalar = Payload.SampleScalar;
			const FVector RBVector = Payload.SampleVector;
			const int32 RBSequence = Payload.Sequence;

			const bool bScalarOK = FMath::IsNearlyEqual(Stats.InjectedScalar, RBScalar, KINDA_SMALL_NUMBER);
			const bool bVectorOK = Stats.InjectedVector.Equals(RBVector, 0.01f);
			const bool bSeqOK = (Stats.InjectedSequence == RBSequence);

			AddProof(Result, bScalarOK,
				FString::Printf(TEXT("[%s] Scalar VERIFIED: injected=%.6f, readback=%.6f (seed=%d)"),
					*RecvLabel, Stats.InjectedScalar, RBScalar, Stats.RandomSeed));

			AddProof(Result, bVectorOK,
				FString::Printf(TEXT("[%s] Vector VERIFIED: injected=%s, readback=%s"),
					*RecvLabel, *Stats.InjectedVector.ToString(), *RBVector.ToString()));

			AddProof(Result, bSeqOK,
				FString::Printf(TEXT("[%s] Sequence VERIFIED: injected=%d, readback=%d"),
					*RecvLabel, Stats.InjectedSequence, RBSequence));

			// Record net stats from this receiver
			const int32 RecvNetReceiveCount = ReadBackPerfOV->GetPerfNetReceiveCount();
			const int32 RecvNetSendCount = ReadBackPerfOV->GetPerfNetSendCount();

			if (bCrossWorldVerification)
			{
				if (bExpectNetReceiveCountPerReceiver.IsValidIndex(i) && bExpectNetReceiveCountPerReceiver[i])
				{
					// S2C / S2MC, or C2MC second-client: standard UE property replication
					// fires OnRep so NetReceiveCount is the authoritative arrival signal.
					AddProof(Result, RecvNetReceiveCount > BaselineReceiveCounts[i],
						FString::Printf(TEXT("[%s] NetReceiveCount=%d (baseline=%d, expected increment)"),
							*RecvLabel, RecvNetReceiveCount, BaselineReceiveCounts[i]));
				}
				else
				{
					// C2S / C2MC relay receiver: relay RPC applies properties directly without
					// UE property replication, so OnRep does not fire and NetReceiveCount stays
					// at baseline. Value match (scalar/vector/seq) drives pass/fail here.
					Result.AddNote(FString::Printf(
						TEXT("[%s] NetReceiveCount=%d (C2S relay path, OnRep not fired; value match drives pass/fail)"),
						*RecvLabel, RecvNetReceiveCount));
				}
			}
			else
			{
				AddProof(Result, RecvNetReceiveCount > 0 || !Stats.Fingerprint.IsRealNetEnvironment(),
					FString::Printf(TEXT("[%s] NetReceiveCount=%d (single-world fallback, PIE=%s)"),
						*RecvLabel, RecvNetReceiveCount,
						Stats.Fingerprint.IsRealNetEnvironment() ? TEXT("YES") : TEXT("NO")));
			}

			Result.AddMetric(FString::Printf(TEXT("%s.netReceiveCount"), *RecvLabel), static_cast<double>(RecvNetReceiveCount));
			Result.AddMetric(FString::Printf(TEXT("%s.netSendCount"), *RecvLabel), static_cast<double>(RecvNetSendCount));

			// Use the last receiver's values for the aggregate stats
			Stats.ReadBackScalar = RBScalar;
			Stats.ReadBackVector = RBVector;
			Stats.ReadBackSequence = RBSequence;
			Stats.bScalarMatch = bScalarOK;
			Stats.bVectorMatch = bVectorOK;
			Stats.bSequenceMatch = bSeqOK;
			Stats.NetReceiveCount = RecvNetReceiveCount;
			Stats.NetSendCount = RecvNetSendCount;
			Stats.StimulusCount = ReadBackPerfOV->GetPerfReplicationStimulusCount();

			if (bScalarOK && bVectorOK && bSeqOK) ++ReceiversVerified;
		}

		// -- Finalize --
		Stats.OperationEndSeconds = FPlatformTime::Seconds();
		Stats.TotalLatencySeconds = Stats.GetDurationSeconds();

		Result.AddNote(FString::Printf(TEXT("PROOF: [%s] Execution started on %s (HasAuthority=%s) -> Replicate to %s"),
			Label, *Origin, bSenderIsServer ? TEXT("true") : TEXT("false"), *Target));
		Result.AddNote(FString::Printf(TEXT("PROOF: [%s] %d/%d receivers verified (crossWorld=%s)"),
			Label, ReceiversVerified, ReceiverControllers.Num(),
			bCrossWorldVerification ? TEXT("YES") : TEXT("NO")));
		Result.AddNote(FString::Printf(TEXT("PROOF: [%s] Random value (seed=%d, scalar=%.6f) injected and VERIFIED via cross-world TryGetValue"),
			Label, Stats.RandomSeed, Stats.InjectedScalar));
		Result.AddNote(FString::Printf(TEXT("STATS: [%s] %s"), Label, *Stats.BuildReport(Label)));

		Result.AddMetric(FString::Printf(TEXT("%s.receiversVerified"), Label), static_cast<double>(ReceiversVerified));
		Result.AddMetric(FString::Printf(TEXT("%s.receiversTotal"), Label), static_cast<double>(ReceiverControllers.Num()));
		Result.AddMetric(FString::Printf(TEXT("%s.crossWorld"), Label), bCrossWorldVerification ? 1.0 : 0.0);

		Stats.EmitMetrics(Result, FString(Label));
		LogCapture.ApplyToResult(Result);
		Result.LogCapture = MoveTemp(LogCapture.FullLogLines);
		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakePropertyRep_ServerToClient()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("AutoReplication.Comprehensive.Property.ServerToClient");
		D.DisplayName  = TEXT("Property Rep: Server -> Client (Cross-World)");
		D.Description  = TEXT("Injects a random scalar+vector on the SERVER world, ticks replication, reads back on the CLIENT world via TryGetValue and verifies exact values.");
		D.Tags         = { TEXT("property"), TEXT("replication"), TEXT("server-to-client"), TEXT("comprehensive"), TEXT("value-verification"), TEXT("cross-world") };
		D.Priority     = 140;
		D.Runner = [](const FGorgeousInsightScenarioContext& Ctx) { return RunPropertyReplication(Ctx, TEXT("PropS2C"), true, false); };
		return D;
	}

	static FGorgeousInsightScenarioDescriptor MakePropertyRep_ServerToMulticast()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("AutoReplication.Comprehensive.Property.ServerToMulticast");
		D.DisplayName  = TEXT("Property Rep: Server -> Multicast (Cross-World)");
		D.Description  = TEXT("Injects a random scalar+vector on the SERVER world, ticks replication, reads back on ALL CLIENT worlds via TryGetValue and verifies exact values on each.");
		D.Tags         = { TEXT("property"), TEXT("replication"), TEXT("server-to-multicast"), TEXT("comprehensive"), TEXT("value-verification"), TEXT("cross-world") };
		D.Priority     = 139;
		D.Runner = [](const FGorgeousInsightScenarioContext& Ctx) { return RunPropertyReplication(Ctx, TEXT("PropS2MC"), true, true); };
		return D;
	}

	static FGorgeousInsightScenarioDescriptor MakePropertyRep_ClientToServer()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("AutoReplication.Comprehensive.Property.ClientToServer");
		D.DisplayName  = TEXT("Property Rep: Client -> Server (Cross-World)");
		D.Description  = TEXT("Injects a random scalar+vector on a CLIENT world, ticks replication, reads back on the SERVER world via TryGetValue and verifies exact values.");
		D.Tags         = { TEXT("property"), TEXT("replication"), TEXT("client-to-server"), TEXT("comprehensive"), TEXT("value-verification"), TEXT("cross-world") };
		D.Priority     = 138;
		D.Runner = [](const FGorgeousInsightScenarioContext& Ctx) { return RunPropertyReplication(Ctx, TEXT("PropC2S"), false, false); };
		return D;
	}

	static FGorgeousInsightScenarioDescriptor MakePropertyRep_ClientToMulticast()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("AutoReplication.Comprehensive.Property.ClientToMulticast");
		D.DisplayName  = TEXT("Property Rep: Client -> Multicast (Cross-World)");
		D.Description  = TEXT("Injects a random scalar+vector on a CLIENT world, ticks replication, reads back on ALL OTHER worlds via TryGetValue and verifies exact values on each.");
		D.Tags         = { TEXT("property"), TEXT("replication"), TEXT("client-to-multicast"), TEXT("comprehensive"), TEXT("value-verification"), TEXT("cross-world") };
		D.Priority     = 137;
		D.Runner = [](const FGorgeousInsightScenarioContext& Ctx) { return RunPropertyReplication(Ctx, TEXT("PropC2MC"), false, true); };
		return D;
	}

	// =====================================================================
	// SECTION 6b: Async RPC Action with Return-Value & Per-Responder Tests
	//
	//   Exercises UGorgeousAutoReplicationRPCRequestAsyncAction through the
	//   complete lifecycle:
	//
	//   A) Factory creation & parameter verification
	//   B) Live activation via PIE PlayerController
	//   C) OnSingleResponderCompleted per-client callback with progress
	//   D) OnCompleted with aggregated FGorgeousAutoReplicationRPCAsyncResult
	//   E) UGorgeousRPC_OV return-value container validation
	//      - GetCachedTargetVariable(), GetCachedResults(), GetCachedResultMap()
	//      - GetArgumentByName() for round-trip payload verification
	//   F) Full async node input matrix (6 types x 3 kinds) through the
	//      async action instead of the direct queue/dequeue path
	//
	//   Because the listen-server path handles server-targeted RPCs
	//   synchronously, CachedResults and the ResultContainer are populated
	//   immediately after Activate() returns, making deterministic
	//   assertions possible inside synchronous test runners.
	// =====================================================================

	/**
	 * Core runner for a single async-action RPC test.
	 * Creates the action, binds the test-helper sink,
	 * activates, and verifies synchronous completion on the listen-server.
	 */
	static FGorgeousInsightScenarioResult RunAsyncRPCActionTest(
		const FGorgeousInsightScenarioContext& Context,
		const TCHAR* Label,
		EGorgeousAutoReplicationRPCType RPCType,
		EGorgeousAutoReplicationTargetKind TargetKind = EGorgeousAutoReplicationTargetKind::EAuto,
		FName Key = DefaultEntryKey)
	{
		FGorgeousInsightScenarioResult Result;
		FGorgeousInsightScopedLogCapture LogCapture;
		FGorgeousPerfReplicationStats Stats;
		Stats.OperationStartSeconds = FPlatformTime::Seconds();

		UWorld* World = ResolveWorld(Context);
		if (!World)
		{
			Result.AddWarning(TEXT("No play world active -- requires PIE or Gauntlet."));
			return Result;
		}

		// -- PIE environment proof --
		Stats.Fingerprint = EmbedEnvironmentProof(World, Result, Label);

		AGorgeousPlayerController* Controller = ResolveController(World);
		if (!Controller)
		{
			Result.AddError(TEXT("No GorgeousPlayerController found."));
			return Result;
		}

		const bool bIsServer = Controller->HasAuthority();
		const FString CallerRole = RoleLabel(bIsServer);
		const FString ExpectedDirection = GetExpectedDirection(RPCType);
		const int32 Sequence = Context.VariantIndex + 1;

		// -- Generate random test values for round-trip verification --
		Stats.RandomSeed = FMath::RandRange(1, 999999);
		FRandomStream RandStream(Stats.RandomSeed);
		Stats.RPCInputValue = RandStream.RandRange(-50000, 50000);
		Stats.RPCInputString = GenerateRandomString(RandStream, 24);
		Stats.RPCExpectedOutput = Stats.RPCInputValue * RPC_TRANSFORM_MULTIPLY + RPC_TRANSFORM_ADD;

		// ── Pre-req: register an OV under the entry key so that ResolveVariableForKey()
		//   returns a candidate for the EAuto / EObjectVariable dispatch paths.
		//   Without this, EAuto falls through to EActorComponent, finds nothing on the PC's
		//   component list, and emits a NoComponent warning instead of completing.
		SetupAsyncRPCTestOV(Controller, Key);

		// ── All dispatch paths use the unified Automation_HandleRPC_WithReturnOV handler. ──
		// This function exists on:
		//   • UGorgeousAutomationTestOV  → EObjectVariable / EAuto paths
		//   • AGorgeousPlayerController  → EOwner path
		//   • UGorgeousAutoReplicationRPCRelayComponent → EActorComponent path
		// Parameter names match BuildVerifiablePayload exactly so the argument-bridge fills
		// them without error.  The backend constructs a UInteger_SOV* as the first param,
		// the handler sets Value = TestInputInt * 7 + 42, and E9 verifies the round-trip.
		constexpr const TCHAR* EffectiveHandlerName = TEXT("Automation_HandleRPC_WithReturnOV");

		// -- Build payload with random values --
		const FGorgeousRPCPayload Payload = BuildVerifiablePayload(
			Controller, EffectiveHandlerName, Sequence, bIsServer, Stats.RPCInputValue, Stats.RPCInputString);
		Stats.PayloadArgumentCount = Payload.Arguments.Num();

		// ── A) Create the async action via the static factory ──
		const double PreCreateTime = FPlatformTime::Seconds();
		UGorgeousAutoReplicationRPCRequestAsyncAction* Action =
			UGorgeousAutoReplicationRPCRequestAsyncAction::RequestAutoReplicationRPC(
				Controller, Key, RPCType, Payload, TargetKind);
		const double PostCreateTime = FPlatformTime::Seconds();

		AddProof(Result, Action != nullptr,
			FString::Printf(TEXT("[%s] Async action created via factory (%.6fs)"), Label, PostCreateTime - PreCreateTime));
		if (!Action)
		{
			Stats.OperationEndSeconds = FPlatformTime::Seconds();
			Stats.EmitMetrics(Result, FString(Label));
			return Result;
		}

		// ── B) Bind the delegate-sink helper BEFORE activation ──
		UGorgeousAsyncActionTestHelper* Sink = NewObject<UGorgeousAsyncActionTestHelper>();
		Sink->BindTo(Action);
		AddProof(Result, true, FString::Printf(TEXT("[%s] Delegate sink bound (OnCompleted, OnFailed, OnSingleResponderCompleted)"), Label));

		// ── C) Activate, on listen-server, server RPCs complete synchronously ──
		const double PreActivateTime = FPlatformTime::Seconds();
		Action->Activate();
		const double PostActivateTime = FPlatformTime::Seconds();
		Stats.QueueLatencySeconds = PostActivateTime - PreActivateTime;

		Result.AddNote(FString::Printf(TEXT("[%s] Activate() completed in %.6fs from %s -> %s (%s, TK=%s)"),
			Label, Stats.QueueLatencySeconds, *CallerRole, *ExpectedDirection,
			IsReliable(RPCType) ? TEXT("reliable") : TEXT("unreliable"),
			*TargetKindToString(TargetKind)));

		// ── D) Check per-responder callback (OnSingleResponderCompleted) ──
		const bool bGotSingleCallbacks = Sink->SingleResponderCallCount > 0;
		if (bGotSingleCallbacks)
		{
			AddProof(Result, true,
				FString::Printf(TEXT("[%s] OnSingleResponderCompleted fired %d time(s)"),
					Label, Sink->SingleResponderCallCount));

			// Verify progressive counting in per-responder results
			for (int32 i = 0; i < Sink->ReceivedSingleResults.Num(); ++i)
			{
				const FGorgeousAutoReplicationRPCAsyncResult& SR = Sink->ReceivedSingleResults[i];
				const FString ResponderKey = SR.Result.Responder.GetStableKey();

				AddProof(Result, SR.TotalReceivedResponders == (i + 1),
					FString::Printf(TEXT("[%s] Per-responder[%d] TotalReceived=%d (expected %d)"),
						Label, i, SR.TotalReceivedResponders, i + 1));

				AddProof(Result, SR.TotalExpectedResponders >= SR.TotalReceivedResponders,
					FString::Printf(TEXT("[%s] Per-responder[%d] Expected=%d >= Received=%d"),
						Label, i, SR.TotalExpectedResponders, SR.TotalReceivedResponders));

				AddProof(Result, !ResponderKey.IsEmpty(),
					FString::Printf(TEXT("[%s] Per-responder[%d] ResponderKey='%s' (non-empty)"),
						Label, i, *ResponderKey));

				Result.AddNote(FString::Printf(TEXT("PROOF: [%s] Per-responder[%d]: Key='%s', Received=%d/%d, bIsLast=%s"),
					Label, i, *ResponderKey, SR.TotalReceivedResponders, SR.TotalExpectedResponders,
					SR.bIsLastResult ? TEXT("true") : TEXT("false")));
			}

			// The last per-responder entry should have bIsLastResult == true (if complete)
			if (Sink->CompletedCallCount > 0)
			{
				const bool bLastMarked = Sink->HasReceivedLastResponder();
				AddProof(Result, bLastMarked,
					FString::Printf(TEXT("[%s] Final per-responder entry has bIsLastResult=true"), Label));
			}
		}
		else
		{
			Result.AddNote(FString::Printf(TEXT("[%s] OnSingleResponderCompleted did not fire synchronously (expected for client-targeted RPCs on server)."), Label));
		}

		// ── E) Check OnCompleted / OnFailed callbacks ──
		const bool bCompleted = Sink->CompletedCallCount > 0;
		const bool bFailed = Sink->FailedCallCount > 0;

		if (bCompleted)
		{
			AddProof(Result, true,
				FString::Printf(TEXT("[%s] OnCompleted fired (sync completion on listen-server)"), Label));

			const FGorgeousAutoReplicationRPCAsyncResult& AsyncResult = Sink->LastCompletedResult;

			// ── E1) Verify ResultContainer (UGorgeousRPC_OV) ──
			const bool bHasContainer = (AsyncResult.ResultContainer != nullptr);
			AddProof(Result, bHasContainer,
				FString::Printf(TEXT("[%s] ResultContainer (UGorgeousRPC_OV*) is %s"),
					Label, bHasContainer ? TEXT("VALID") : TEXT("NULL")));

			if (bHasContainer)
			{
				UGorgeousRPC_OV* Container = AsyncResult.ResultContainer;
				Container->SetNetworkingEnabled(true); // Enable networking on the OV for TryGetValue and related calls

				// Verify HasResult()
				AddProof(Result, Container->HasResult(),
					FString::Printf(TEXT("[%s] ResultContainer->HasResult() == true"), Label));

				// ── E2) Verify return-value OV via GetCachedTargetVariable() ──
				UGorgeousObjectVariable* TargetVar = Container->GetCachedTargetVariable();
				AddProof(Result, TargetVar != nullptr,
					FString::Printf(TEXT("[%s] ResultContainer->GetCachedTargetVariable() is %s"),
						Label, TargetVar ? TEXT("VALID") : TEXT("NULL")));

				// ── E3) Verify CachedResults array ──
				const TArray<FGorgeousAutoReplicationRPCResult>& Results = Container->GetCachedResults();
				AddProof(Result, Results.Num() > 0,
					FString::Printf(TEXT("[%s] ResultContainer->GetCachedResults() contains %d entries"),
						Label, Results.Num()));

				// ── E4) Verify CachedResultMap ──
				const TMap<FString, FGorgeousAutoReplicationRPCResult>& ResultMap = Container->GetCachedResultMap();
				AddProof(Result, ResultMap.Num() > 0,
					FString::Printf(TEXT("[%s] ResultContainer->GetCachedResultMap() contains %d entries"),
						Label, ResultMap.Num()));

				// ── E5) List all responder keys in the result map ──
				for (const auto& KVP : ResultMap)
				{
					Result.AddNote(FString::Printf(TEXT("PROOF: [%s] ResultMap entry: ResponderKey='%s', TargetKind=%s, HasTargetVar=%s"),
						Label, *KVP.Key,
						*TargetKindToString(KVP.Value.TargetKind),
						KVP.Value.TargetVariable ? TEXT("Yes") : TEXT("No")));
				}

				// ── E6) Verify payload argument round-trip via GetArgumentContainerByName() ──
				{
					FGorgeousRPCArgumentContainer IntArgContainer;
					if (Container->GetArgumentContainerByName(TEXT("TestInputInt"), IntArgContainer)
						&& IntArgContainer.ValueBytes.Num() == sizeof(int32))
					{
						int32 IntVal = 0;
						FMemory::Memcpy(&IntVal, IntArgContainer.ValueBytes.GetData(), sizeof(int32));
						Stats.RPCActualOutput = IntVal;
						Stats.bRPCValueMatch = (IntVal == Stats.RPCInputValue);
						AddProof(Result, Stats.bRPCValueMatch,
							FString::Printf(TEXT("[%s] ResultContainer arg 'TestInputInt' VERIFIED: sent=%d, returned=%d (match=%s)"),
								Label, Stats.RPCInputValue, IntVal, Stats.bRPCValueMatch ? TEXT("YES") : TEXT("NO")));

						// Apply mathematical transformation
						if (Stats.bRPCValueMatch)
						{
							const int32 Transformed = IntVal * RPC_TRANSFORM_MULTIPLY + RPC_TRANSFORM_ADD;
							AddProof(Result, Transformed == Stats.RPCExpectedOutput,
								FString::Printf(TEXT("[%s] Transformation: %d * %d + %d = %d (expected %d)"),
									Label, IntVal, RPC_TRANSFORM_MULTIPLY, RPC_TRANSFORM_ADD,
									Transformed, Stats.RPCExpectedOutput));
							Stats.RPCActualOutput = Transformed;
						}
					}
					else
					{
						Result.AddNote(FString::Printf(TEXT("[%s] ResultContainer->GetArgumentContainerByName('TestInputInt') returned null, payload may have been consumed differently."), Label));
					}
				}

				{
					FGorgeousRPCArgumentContainer StrArgContainer;
					if (Container->GetArgumentContainerByName(TEXT("TestInputString"), StrArgContainer)
						&& StrArgContainer.ValueBytes.Num() > 0)
					{
						FString StrVal;
						FMemoryReader Reader(StrArgContainer.ValueBytes);
						Reader << StrVal;
						Stats.RPCReadBackString = StrVal;
						Stats.bRPCStringMatch = (StrVal == Stats.RPCInputString);
						AddProof(Result, Stats.bRPCStringMatch,
							FString::Printf(TEXT("[%s] ResultContainer arg 'TestInputString' VERIFIED: sent='%s', returned='%s' (match=%s)"),
								Label, *Stats.RPCInputString, *StrVal, Stats.bRPCStringMatch ? TEXT("YES") : TEXT("NO")));
					}
					else
					{
						Result.AddNote(FString::Printf(TEXT("[%s] ResultContainer->GetArgumentContainerByName('TestInputString') returned null."), Label));
					}
				}
			}

			// ── E7) Verify ResultSet array in the async result ──
			AddProof(Result, AsyncResult.ResultSet.Num() > 0,
				FString::Printf(TEXT("[%s] AsyncResult.ResultSet contains %d entries"),
					Label, AsyncResult.ResultSet.Num()));

			// ── E8) Verify ResultMap in the async result ──
			AddProof(Result, AsyncResult.ResultMap.Num() > 0,
				FString::Printf(TEXT("[%s] AsyncResult.ResultMap contains %d entries"),
					Label, AsyncResult.ResultMap.Num()));

			// ── E9) Verify handler-written return OV (first-parameter OV pattern) ──
			// All tests use Automation_HandleRPC_WithReturnOV which sets:
			//   ReturnValue->Value = TestInputInt * RPC_TRANSFORM_MULTIPLY + RPC_TRANSFORM_ADD
			// The backend constructs a UInteger_SOV*, the handler populates it, and after
			// ExecuteAutoReplicationRPC the OV becomes the TargetVariable of the result container.
			{
				UGorgeousObjectVariable* TVarRaw = AsyncResult.ResultContainer
					? AsyncResult.ResultContainer->GetCachedTargetVariable()
					: nullptr;
				AddProof(Result, TVarRaw != nullptr,
					FString::Printf(TEXT("[%s] E9: GetCachedTargetVariable() is %s (expected non-null UInteger_SOV)"),
						Label, TVarRaw ? *TVarRaw->GetClass()->GetName() : TEXT("null")));
				if (UInteger_SOV* ReturnIntOV = Cast<UInteger_SOV>(TVarRaw))
				{
					const int32 Expected = Stats.RPCExpectedOutput;
					Stats.bRPCValueMatch = (ReturnIntOV->Value == Expected);
					Stats.RPCActualOutput = ReturnIntOV->Value;
					AddProof(Result, Stats.bRPCValueMatch,
						FString::Printf(TEXT("[%s] E9: ReturnOV.Value VERIFIED: input=%d * %d + %d = expected=%d, got=%d (match=%s)"),
							Label, Stats.RPCInputValue, RPC_TRANSFORM_MULTIPLY, RPC_TRANSFORM_ADD,
							Expected, ReturnIntOV->Value,
							Stats.bRPCValueMatch ? TEXT("YES") : TEXT("NO")));
				}
				else if (TVarRaw)
				{
					Result.AddNote(FString::Printf(TEXT("[%s] E9: TargetVariable is %s, not UInteger_SOV, handler may not have been invoked."),
						Label, *TVarRaw->GetClass()->GetName()));
				}
			}
		}
		else if (bFailed)
		{
			// Failure is acceptable for client-targeted RPCs when running on the server
			// (the mixin may lack the route to a connected client).
			Result.AddNote(FString::Printf(TEXT("[%s] OnFailed fired, mixin could not dispatch %s from %s (expected when target is unreachable)."),
				Label, *RPCTypeToString(RPCType), *CallerRole));
		}
		else
		{
			// Neither completed nor failed synchronously, the RPC is pending async delivery
			Result.AddNote(FString::Printf(TEXT("[%s] RPC dispatched but not yet completed synchronously (pending async delivery for %s -> %s)."),
				Label, *CallerRole, *ExpectedDirection));
		}

		// ── F) Verify the action's public properties directly ──
		const TArray<FGorgeousAutoReplicationRPCResult>& ActionResults = Action->GetResolvedResults();
		const TMap<FString, FGorgeousAutoReplicationRPCResult>& ActionResultMap = Action->GetResolvedResultMap();

		if (ActionResults.Num() > 0)
		{
			AddProof(Result, true,
				FString::Printf(TEXT("[%s] Action->GetResolvedResults() has %d entries"), Label, ActionResults.Num()));

			// Verify the primary result's metadata
			const FGorgeousAutoReplicationRPCResult& PrimaryResult = Action->GetResolvedResult();
			AddProof(Result, PrimaryResult.QueuedRPC.Key == Key,
				FString::Printf(TEXT("[%s] Primary result key=%s matches request key=%s"),
					Label, *PrimaryResult.QueuedRPC.Key.ToString(), *Key.ToString()));
			AddProof(Result, PrimaryResult.QueuedRPC.Type == RPCType,
				FString::Printf(TEXT("[%s] Primary result type=%s matches request type=%s"),
					Label, *RPCTypeToString(PrimaryResult.QueuedRPC.Type), *RPCTypeToString(RPCType)));
		}

		if (ActionResultMap.Num() > 0)
		{
			AddProof(Result, true,
				FString::Printf(TEXT("[%s] Action->GetResolvedResultMap() has %d entries"), Label, ActionResultMap.Num()));
		}

		UGorgeousObjectVariable* ResolvedTargetVar = Action->GetResolvedTargetVariable();
		if (ResolvedTargetVar)
		{
			AddProof(Result, true,
				FString::Printf(TEXT("[%s] Action->GetResolvedTargetVariable() = %s"), Label, *ResolvedTargetVar->GetClass()->GetName()));
		}

		// -- Finalize stats --
		Stats.OperationEndSeconds = FPlatformTime::Seconds();
		Stats.TotalLatencySeconds = Stats.GetDurationSeconds();
		Stats.DispatchLatencySeconds = Stats.QueueLatencySeconds; // activation == dispatch for async actions

		Result.AddNote(FString::Printf(TEXT("PROOF: [%s] Async action: %s -> %s | Reliable=%s | TK=%s | IntArg=%d | StrArg='%s' | Completed=%s | Failed=%s | SingleResponders=%d"),
			Label, *CallerRole, *ExpectedDirection,
			IsReliable(RPCType) ? TEXT("Yes") : TEXT("No"),
			*TargetKindToString(TargetKind),
			Stats.RPCInputValue, *Stats.RPCInputString,
			bCompleted ? TEXT("Yes") : TEXT("No"),
			bFailed ? TEXT("Yes") : TEXT("No"),
			Sink->SingleResponderCallCount));

		Result.AddNote(FString::Printf(TEXT("STATS: [%s] %s"), Label, *Stats.BuildReport(Label)));
		Stats.EmitMetrics(Result, FString(Label));

		LogCapture.ApplyToResult(Result);
		Result.LogCapture = MoveTemp(LogCapture.FullLogLines);

		// ── Backend dispatch failure guard ──────────────────────────────────────
		// Warnings like NoComponent / NoVariable / NoOwnerHandler mean the RPC was
		// silently dropped without calling OnCompleted or OnFailed.  The test would
		// otherwise report Success:true because no AddProof(false) was reached.
		// Promote these captured warnings to hard errors so bSuccess becomes false.
		{
			static const TArray<FString> BackendFailureTags = {
				TEXT("NoComponent"), TEXT("NoVariable"), TEXT("NoOwnerHandler")
			};
			TArray<FString> ToPromote;
			for (const FString& W : Result.Warnings)
			{
				for (const FString& Tag : BackendFailureTags)
				{
					if (W.Contains(Tag))
					{
						ToPromote.Add(W);
						break;
					}
				}
			}
			for (const FString& P : ToPromote)
			{
				Result.Warnings.Remove(P);
				Result.AddError(FString::Printf(TEXT("[%s] RPC dispatch failed, backend warning elevated to error: %s"), Label, *P));
			}
		}

		return Result;
	}

	// -- 6b-1: Individual async-action scenarios (per RPC type, reliable server path) --

#define MAKE_ASYNC_ACTION_SCENARIO(SuffixName, DisplayStr, RPCTypeVal, KindVal, TagDir, TagRel, InPriority) \
	static FGorgeousInsightScenarioDescriptor MakeAsyncAction_##SuffixName() \
	{ \
		FGorgeousInsightScenarioDescriptor D; \
		D.ScenarioName = TEXT("AutoReplication.Comprehensive.AsyncAction." #SuffixName); \
		D.DisplayName  = TEXT(DisplayStr); \
		D.Description  = TEXT("Async action test: " #SuffixName ", factory, bind, activate, per-responder callback, return-value OV verification."); \
		D.Tags         = { TEXT("rpc"), TEXT("async-action"), TEXT("return-value"), TEXT(TagDir), TEXT(TagRel), TEXT("comprehensive"), TEXT("value-verification") }; \
		D.Priority     = InPriority; \
		D.Runner = [](const FGorgeousInsightScenarioContext& Ctx) { \
			return RunAsyncRPCActionTest(Ctx, TEXT("Async_" #SuffixName), RPCTypeVal, KindVal); \
		}; \
		return D; \
	}

	// Server RPCs (most likely to complete synchronously on listen-server)
	MAKE_ASYNC_ACTION_SCENARIO(S2C_Reliable_Auto,    "Async: S->C Reliable Auto (ReturnVal)",       EGorgeousAutoReplicationRPCType::EReliableClient,      EGorgeousAutoReplicationTargetKind::EAuto,           "server-to-client",    "reliable",   100)
	MAKE_ASYNC_ACTION_SCENARIO(S2C_Unreliable_Auto,   "Async: S->C Unreliable Auto (ReturnVal)",     EGorgeousAutoReplicationRPCType::EUnreliableClient,    EGorgeousAutoReplicationTargetKind::EAuto,           "server-to-client",    "unreliable", 99)
	MAKE_ASYNC_ACTION_SCENARIO(C2S_Reliable_Auto,     "Async: C->S Reliable Auto (ReturnVal)",       EGorgeousAutoReplicationRPCType::EReliableServer,      EGorgeousAutoReplicationTargetKind::EAuto,           "client-to-server",    "reliable",   98)
	MAKE_ASYNC_ACTION_SCENARIO(C2S_Unreliable_Auto,   "Async: C->S Unreliable Auto (ReturnVal)",     EGorgeousAutoReplicationRPCType::EUnreliableServer,    EGorgeousAutoReplicationTargetKind::EAuto,           "client-to-server",    "unreliable", 97)
	MAKE_ASYNC_ACTION_SCENARIO(S2MC_Reliable_Auto,    "Async: S->MC Reliable Auto (ReturnVal)",      EGorgeousAutoReplicationRPCType::EReliableMulticast,   EGorgeousAutoReplicationTargetKind::EAuto,           "server-to-multicast", "reliable",   96)
	MAKE_ASYNC_ACTION_SCENARIO(S2MC_Unreliable_Auto,  "Async: S->MC Unreliable Auto (ReturnVal)",    EGorgeousAutoReplicationRPCType::EUnreliableMulticast, EGorgeousAutoReplicationTargetKind::EAuto,           "server-to-multicast", "unreliable", 95)
	MAKE_ASYNC_ACTION_SCENARIO(C2MC_Reliable_Auto,    "Async: C->MC Reliable Auto (ReturnVal)",      EGorgeousAutoReplicationRPCType::EReliableMulticast,   EGorgeousAutoReplicationTargetKind::EAuto,           "client-to-multicast", "reliable",   94)
	MAKE_ASYNC_ACTION_SCENARIO(C2MC_Unreliable_Auto,  "Async: C->MC Unreliable Auto (ReturnVal)",    EGorgeousAutoReplicationRPCType::EUnreliableMulticast, EGorgeousAutoReplicationTargetKind::EAuto,           "client-to-multicast", "unreliable", 93)

	// TargetKind variations through the async action
	MAKE_ASYNC_ACTION_SCENARIO(S2C_Reliable_OV,       "Async: S->C Reliable OV (ReturnVal)",         EGorgeousAutoReplicationRPCType::EReliableClient,      EGorgeousAutoReplicationTargetKind::EObjectVariable,  "server-to-client",    "reliable",   92)
	MAKE_ASYNC_ACTION_SCENARIO(C2S_Reliable_OV,       "Async: C->S Reliable OV (ReturnVal)",         EGorgeousAutoReplicationRPCType::EReliableServer,      EGorgeousAutoReplicationTargetKind::EObjectVariable,  "client-to-server",    "reliable",   91)
	MAKE_ASYNC_ACTION_SCENARIO(S2MC_Reliable_OV,      "Async: S->MC Reliable OV (ReturnVal)",        EGorgeousAutoReplicationRPCType::EReliableMulticast,   EGorgeousAutoReplicationTargetKind::EObjectVariable,  "server-to-multicast", "reliable",   90)
	MAKE_ASYNC_ACTION_SCENARIO(S2C_Reliable_Owner,    "Async: S->C Reliable Owner (ReturnVal)",      EGorgeousAutoReplicationRPCType::EReliableClient,      EGorgeousAutoReplicationTargetKind::EOwner,           "server-to-client",    "reliable",   89)
	MAKE_ASYNC_ACTION_SCENARIO(C2S_Reliable_Owner,    "Async: C->S Reliable Owner (ReturnVal)",      EGorgeousAutoReplicationRPCType::EReliableServer,      EGorgeousAutoReplicationTargetKind::EOwner,           "client-to-server",    "reliable",   88)
	MAKE_ASYNC_ACTION_SCENARIO(S2MC_Reliable_Owner,   "Async: S->MC Reliable Owner (ReturnVal)",     EGorgeousAutoReplicationRPCType::EReliableMulticast,   EGorgeousAutoReplicationTargetKind::EOwner,           "server-to-multicast", "reliable",   87)

#undef MAKE_ASYNC_ACTION_SCENARIO

	// -- 6b-1b: Key-variation scenarios through the async action --

#define MAKE_ASYNC_ACTION_KEY_SCENARIO(SuffixName, DisplayStr, RPCTypeVal, KindVal, KeyVal, TagDir, TagRel, InPriority) \
	static FGorgeousInsightScenarioDescriptor MakeAsyncAction_##SuffixName() \
	{ \
		FGorgeousInsightScenarioDescriptor D; \
		D.ScenarioName = TEXT("AutoReplication.Comprehensive.AsyncAction." #SuffixName); \
		D.DisplayName  = TEXT(DisplayStr); \
		D.Description  = TEXT("Async action test with key variation: " #SuffixName ", factory, bind, activate, return-value OV verification."); \
		D.Tags         = { TEXT("rpc"), TEXT("async-action"), TEXT("return-value"), TEXT("key"), TEXT(TagDir), TEXT(TagRel), TEXT("comprehensive"), TEXT("value-verification") }; \
		D.Priority     = InPriority; \
		D.Runner = [](const FGorgeousInsightScenarioContext& Ctx) { \
			return RunAsyncRPCActionTest(Ctx, TEXT("Async_" #SuffixName), RPCTypeVal, KindVal, KeyVal); \
		}; \
		return D; \
	}

	MAKE_ASYNC_ACTION_KEY_SCENARIO(AltKey_S2C_Reliable,    "Async: AltKey S->C Reliable (ReturnVal)",     EGorgeousAutoReplicationRPCType::EReliableClient,      EGorgeousAutoReplicationTargetKind::EAuto, AlternateEntryKey, "server-to-client",    "reliable",   82)
	MAKE_ASYNC_ACTION_KEY_SCENARIO(AltKey_C2S_Reliable,    "Async: AltKey C->S Reliable (ReturnVal)",     EGorgeousAutoReplicationRPCType::EReliableServer,      EGorgeousAutoReplicationTargetKind::EAuto, AlternateEntryKey, "client-to-server",    "reliable",   81)
	MAKE_ASYNC_ACTION_KEY_SCENARIO(AltKey_S2MC_Reliable,   "Async: AltKey S->MC Reliable (ReturnVal)",    EGorgeousAutoReplicationRPCType::EReliableMulticast,   EGorgeousAutoReplicationTargetKind::EAuto, AlternateEntryKey, "server-to-multicast", "reliable",   80)
	MAKE_ASYNC_ACTION_KEY_SCENARIO(AltKey_C2MC_Reliable,   "Async: AltKey C->MC Reliable (ReturnVal)",    EGorgeousAutoReplicationRPCType::EReliableMulticast,   EGorgeousAutoReplicationTargetKind::EAuto, AlternateEntryKey, "client-to-multicast", "reliable",   79)
	MAKE_ASYNC_ACTION_KEY_SCENARIO(AltKey_C2S_Reliable_OV, "Async: AltKey C->S Reliable OV (ReturnVal)",  EGorgeousAutoReplicationRPCType::EReliableServer,      EGorgeousAutoReplicationTargetKind::EObjectVariable, AlternateEntryKey, "client-to-server", "reliable", 78)

#undef MAKE_ASYNC_ACTION_KEY_SCENARIO

	// -- 6b-2: Full async-action input matrix (6 types x 3 kinds = 18 combos) --

	static FGorgeousInsightScenarioResult RunAsyncActionFullMatrix(const FGorgeousInsightScenarioContext& Context)
	{
		FGorgeousInsightScenarioResult Result;
		FGorgeousInsightScopedLogCapture LogCapture;
		const double StartTime = FPlatformTime::Seconds();

		UWorld* World = ResolveWorld(Context);
		if (!World)
		{
			Result.AddWarning(TEXT("No play world active -- requires PIE or Gauntlet."));
			return Result;
		}

		// -- PIE environment proof --
		const FGorgeousPerfEnvironmentFingerprint FP = EmbedEnvironmentProof(World, Result, TEXT("AsyncActionMatrix"));

		AGorgeousPlayerController* Controller = ResolveController(World);
		if (!Controller)
		{
			Result.AddError(TEXT("No GorgeousPlayerController found."));
			return Result;
		}

		const bool bIsServer = Controller->HasAuthority();
		const FString CallerRole = RoleLabel(bIsServer);

		const TArray<EGorgeousAutoReplicationRPCType> AllRPCTypes = {
			EGorgeousAutoReplicationRPCType::EReliableServer,
			EGorgeousAutoReplicationRPCType::EReliableClient,
			EGorgeousAutoReplicationRPCType::EReliableMulticast,
			EGorgeousAutoReplicationRPCType::EUnreliableServer,
			EGorgeousAutoReplicationRPCType::EUnreliableClient,
			EGorgeousAutoReplicationRPCType::EUnreliableMulticast
		};

		const TArray<EGorgeousAutoReplicationTargetKind> AllTargetKinds = {
			EGorgeousAutoReplicationTargetKind::EAuto,
			EGorgeousAutoReplicationTargetKind::EObjectVariable,
			EGorgeousAutoReplicationTargetKind::EOwner
		};

		int32 Total = 0, Completed = 0, Failed = 0, Pending = 0;
		int32 SingleCallbacksFired = 0, ReturnValueVerified = 0, ContainerValid = 0;
		double TotalActivateTime = 0.0;
		FRandomStream MatrixRand(FMath::RandRange(1, 999999));

		for (EGorgeousAutoReplicationRPCType RPCType : AllRPCTypes)
		{
			for (EGorgeousAutoReplicationTargetKind Kind : AllTargetKinds)
			{
				++Total;
				const FString ComboLabel = FString::Printf(TEXT("AM_%s+%s"), *RPCTypeToString(RPCType), *TargetKindToString(Kind));
				const int32 ComboRandInt = MatrixRand.RandRange(-10000, 10000);
				const FString ComboRandStr = GenerateRandomString(MatrixRand, 8);

				const FGorgeousRPCPayload Payload = BuildVerifiablePayload(
					Controller, *ComboLabel, Total, bIsServer, ComboRandInt, ComboRandStr);

				// Create async action
				UGorgeousAutoReplicationRPCRequestAsyncAction* Action =
					UGorgeousAutoReplicationRPCRequestAsyncAction::RequestAutoReplicationRPC(
						Controller, DefaultEntryKey, RPCType, Payload, Kind);

				if (!Action)
				{
					++Failed;
					Result.AddError(FString::Printf(TEXT("[%s] Factory returned nullptr"), *ComboLabel));
					continue;
				}

				// Bind delegate sink
				UGorgeousAsyncActionTestHelper* Sink = NewObject<UGorgeousAsyncActionTestHelper>();
				Sink->BindTo(Action);

				// Activate
				const double T0 = FPlatformTime::Seconds();
				Action->Activate();
				TotalActivateTime += FPlatformTime::Seconds() - T0;

				// Evaluate result
				if (Sink->CompletedCallCount > 0)
				{
					++Completed;
					SingleCallbacksFired += Sink->SingleResponderCallCount;

					// Check ResultContainer
					if (Sink->LastCompletedResult.ResultContainer != nullptr)
					{
						++ContainerValid;
						UGorgeousRPC_OV* Container = Sink->LastCompletedResult.ResultContainer;
						Container->SetNetworkingEnabled(true); // Enable networking on the OV for TryGetValue and related calls

						// Try to verify the int argument
						FGorgeousRPCArgumentContainer IntArgContainer;
						if (Container->GetArgumentContainerByName(TEXT("TestInputInt"), IntArgContainer)
							&& IntArgContainer.ValueBytes.Num() == sizeof(int32))
						{
							int32 IntVal = 0;
							FMemory::Memcpy(&IntVal, IntArgContainer.ValueBytes.GetData(), sizeof(int32));
							if (IntVal == ComboRandInt)
							{
								++ReturnValueVerified;
							}
						}
					}

					Result.AddNote(FString::Printf(TEXT("PROOF: [%s] Async COMPLETED from %s -> %s | TK=%s | int=%d | SingleCallbacks=%d | ContainerValid=%s"),
						*ComboLabel, *CallerRole, *GetExpectedDirection(RPCType), *TargetKindToString(Kind),
						ComboRandInt, Sink->SingleResponderCallCount,
						Sink->LastCompletedResult.ResultContainer ? TEXT("Yes") : TEXT("No")));
				}
				else if (Sink->FailedCallCount > 0)
				{
					++Failed;
					Result.AddNote(FString::Printf(TEXT("[%s] Async FAILED (mixin could not dispatch %s from %s)"),
						*ComboLabel, *RPCTypeToString(RPCType), *CallerRole));
				}
				else
				{
					++Pending;
					Result.AddNote(FString::Printf(TEXT("[%s] Async PENDING (not completed synchronously)"),
						*ComboLabel));
				}
			}
		}

		const double EndTime = FPlatformTime::Seconds();

		// Metrics
		Result.AddMetric(TEXT("asyncMatrix.total"), static_cast<double>(Total));
		Result.AddMetric(TEXT("asyncMatrix.completed"), static_cast<double>(Completed));
		Result.AddMetric(TEXT("asyncMatrix.failed"), static_cast<double>(Failed));
		Result.AddMetric(TEXT("asyncMatrix.pending"), static_cast<double>(Pending));
		Result.AddMetric(TEXT("asyncMatrix.singleCallbacksFired"), static_cast<double>(SingleCallbacksFired));
		Result.AddMetric(TEXT("asyncMatrix.returnValueVerified"), static_cast<double>(ReturnValueVerified));
		Result.AddMetric(TEXT("asyncMatrix.containerValid"), static_cast<double>(ContainerValid));
		Result.AddMetric(TEXT("asyncMatrix.totalActivateTimeS"), TotalActivateTime, TEXT("s"));
		Result.AddMetric(TEXT("asyncMatrix.avgActivateTimeS"), TotalActivateTime / FMath::Max(1, Total), TEXT("s"));
		Result.AddMetric(TEXT("asyncMatrix.totalDurationS"), EndTime - StartTime, TEXT("s"));

		AddProof(Result, (Completed + Pending) > 0,
			FString::Printf(TEXT("Async action matrix: %d/%d completed, %d pending, %d failed, %d return-values verified, %d containers valid, %d single-responder callbacks from %s in %.4fs"),
				Completed, Total, Pending, Failed, ReturnValueVerified, ContainerValid, SingleCallbacksFired, *CallerRole, EndTime - StartTime));

		LogCapture.ApplyToResult(Result);
		Result.LogCapture = MoveTemp(LogCapture.FullLogLines);
		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakeAsyncActionFullMatrix()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("AutoReplication.Comprehensive.AsyncAction.FullMatrix");
		D.DisplayName  = TEXT("Async Action: Full Matrix (18 combos, ReturnVal + PerResponder)");
		D.Description  = TEXT("Exercises UGorgeousAutoReplicationRPCRequestAsyncAction with every 6 RPC types x 3 target kinds. Verifies per-responder callback, OnCompleted, and UGorgeousRPC_OV return-value container.");
		D.Tags         = { TEXT("rpc"), TEXT("async-action"), TEXT("return-value"), TEXT("matrix"), TEXT("comprehensive"), TEXT("value-verification") };
		D.Priority     = 86;
		D.Runner       = &RunAsyncActionFullMatrix;
		return D;
	}

	// -- 6b-3: Staggered multi-responder arrival simulation --
	//
	// This scenario verifies that OnSingleResponderCompleted fires correctly
	// when multiple responders are registered and results arrive at different
	// times.  In a listen-server PIE environment with 1 server, one round of
	// RPC triggers exactly 1 responder; this test fires MULTIPLE sequential
	// RPCs with different types and aggregates their per-responder tracking.

	static FGorgeousInsightScenarioResult RunStaggeredResponderTest(const FGorgeousInsightScenarioContext& Context)
	{
		FGorgeousInsightScenarioResult Result;
		FGorgeousInsightScopedLogCapture LogCapture;
		const double StartTime = FPlatformTime::Seconds();

		UWorld* World = ResolveWorld(Context);
		if (!World)
		{
			Result.AddWarning(TEXT("No play world active -- requires PIE or Gauntlet."));
			return Result;
		}

		const FGorgeousPerfEnvironmentFingerprint FP = EmbedEnvironmentProof(World, Result, TEXT("StaggeredResponder"));

		AGorgeousPlayerController* Controller = ResolveController(World);
		if (!Controller)
		{
			Result.AddError(TEXT("No GorgeousPlayerController found."));
			return Result;
		}

		const bool bIsServer = Controller->HasAuthority();

		// Fire 5 sequential reliable-server RPCs, each with a different random value.
		// Track per-responder callback progress across all of them.
		const int32 NumRounds = 5;
		int32 TotalSingleCallbacks = 0;
		int32 TotalCompletions = 0;
		int32 LastResponderCount = 0;
		int32 ValueSameCount = 0;
		FRandomStream RandStream(FMath::RandRange(1, 999999));

		for (int32 Round = 0; Round < NumRounds; ++Round)
		{
			const int32 RoundInt = RandStream.RandRange(-99999, 99999);
			const FString RoundStr = GenerateRandomString(RandStream, 12);
			const FGorgeousRPCPayload Payload = BuildVerifiablePayload(
				Controller, TEXT("StaggeredRound"), Round + 1, bIsServer, RoundInt, RoundStr);

			UGorgeousAutoReplicationRPCRequestAsyncAction* Action =
				UGorgeousAutoReplicationRPCRequestAsyncAction::RequestAutoReplicationRPC(
					Controller, DefaultEntryKey, EGorgeousAutoReplicationRPCType::EReliableServer, Payload,
					EGorgeousAutoReplicationTargetKind::EAuto);

			if (!Action) continue;

			UGorgeousAsyncActionTestHelper* Sink = NewObject<UGorgeousAsyncActionTestHelper>();
			Sink->BindTo(Action);
			Action->Activate();

			TotalSingleCallbacks += Sink->SingleResponderCallCount;
			TotalCompletions += Sink->CompletedCallCount;

			if (Sink->HasReceivedLastResponder()) ++LastResponderCount;

			// Check return value
			if (Sink->CompletedCallCount > 0 && Sink->LastCompletedResult.ResultContainer)
			{
				FGorgeousRPCArgumentContainer IntArgContainer;
				if (Sink->LastCompletedResult.ResultContainer->GetArgumentContainerByName(TEXT("TestInputInt"), IntArgContainer)
					&& IntArgContainer.ValueBytes.Num() == sizeof(int32))
				{
					int32 IntVal = 0;
					FMemory::Memcpy(&IntVal, IntArgContainer.ValueBytes.GetData(), sizeof(int32));
					if (IntVal == RoundInt) ++ValueSameCount;
				}
			}

			Result.AddNote(FString::Printf(TEXT("PROOF: [StaggeredRound %d/%d] int=%d | SingleCallbacks=%d | Completed=%s | bIsLast=%s"),
				Round + 1, NumRounds, RoundInt, Sink->SingleResponderCallCount,
				Sink->CompletedCallCount > 0 ? TEXT("Yes") : TEXT("No"),
				Sink->HasReceivedLastResponder() ? TEXT("Yes") : TEXT("No")));
		}

		const double EndTime = FPlatformTime::Seconds();

		Result.AddMetric(TEXT("staggered.rounds"), static_cast<double>(NumRounds));
		Result.AddMetric(TEXT("staggered.totalSingleCallbacks"), static_cast<double>(TotalSingleCallbacks));
		Result.AddMetric(TEXT("staggered.totalCompletions"), static_cast<double>(TotalCompletions));
		Result.AddMetric(TEXT("staggered.lastResponderCount"), static_cast<double>(LastResponderCount));
		Result.AddMetric(TEXT("staggered.valueSameCount"), static_cast<double>(ValueSameCount));
		Result.AddMetric(TEXT("staggered.durationS"), EndTime - StartTime, TEXT("s"));

		AddProof(Result, TotalCompletions > 0,
			FString::Printf(TEXT("Staggered responder: %d/%d rounds completed, %d single callbacks, %d values verified, %d bIsLast in %.4fs"),
				TotalCompletions, NumRounds, TotalSingleCallbacks, ValueSameCount, LastResponderCount, EndTime - StartTime));

		LogCapture.ApplyToResult(Result);
		Result.LogCapture = MoveTemp(LogCapture.FullLogLines);
		return Result;
	}

	static FGorgeousInsightScenarioDescriptor MakeStaggeredResponder()
	{
		FGorgeousInsightScenarioDescriptor D;
		D.ScenarioName = TEXT("AutoReplication.Comprehensive.AsyncAction.StaggeredResponder");
		D.DisplayName  = TEXT("Async Action: Staggered Multi-Responder (5 rounds)");
		D.Description  = TEXT("Fires 5 sequential RPCs via the async action, tracking per-responder callback progress, bIsLastResponder, and return-value round-trip across all rounds.");
		D.Tags         = { TEXT("rpc"), TEXT("async-action"), TEXT("return-value"), TEXT("staggered"), TEXT("comprehensive"), TEXT("value-verification") };
		D.Priority     = 85;
		D.Runner       = &RunStaggeredResponderTest;
		return D;
	}
}

// =====================================================================
// Registration
// =====================================================================

// Section 1: Transporter Probe Matrix
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeTransporterProbeMatrix());

// Section 2: Property Replication Direction (cross-world readback via TryGetValue + replication flush)
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakePropertyRep_ServerToClient());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakePropertyRep_ServerToMulticast());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakePropertyRep_ClientToServer());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakePropertyRep_ClientToMulticast());

// Section 6b: Async Action with Return-Value & Per-Responder Tests (all RPC directions, target kinds, key variations)
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncAction_S2C_Reliable_Auto());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncAction_S2C_Unreliable_Auto());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncAction_C2S_Reliable_Auto());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncAction_C2S_Unreliable_Auto());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncAction_S2MC_Reliable_Auto());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncAction_S2MC_Unreliable_Auto());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncAction_C2MC_Reliable_Auto());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncAction_C2MC_Unreliable_Auto());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncAction_S2C_Reliable_OV());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncAction_C2S_Reliable_OV());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncAction_S2MC_Reliable_OV());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncAction_S2C_Reliable_Owner());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncAction_C2S_Reliable_Owner());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncAction_S2MC_Reliable_Owner());

// Key-variation async scenarios
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncAction_AltKey_S2C_Reliable());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncAction_AltKey_C2S_Reliable());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncAction_AltKey_S2MC_Reliable());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncAction_AltKey_C2MC_Reliable());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncAction_AltKey_C2S_Reliable_OV());

// Full async-action matrix + staggered responder
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeAsyncActionFullMatrix());
REGISTER_GORGEOUS_INSIGHT_SCENARIO(GorgeousComprehensiveReplicationTests::MakeStaggeredResponder());

#endif // WITH_DEV_AUTOMATION_TESTS
