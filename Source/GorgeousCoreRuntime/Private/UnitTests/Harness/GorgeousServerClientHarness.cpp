#include "UnitTests/Harness/GorgeousServerClientHarness.h"

#include "Engine/Engine.h"
#include "Engine/GameEngine.h"
#include "Engine/Level.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Engine/PendingNetGame.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "QualityOfLife/GorgeousPlayerController.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTLS.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/ScopeExit.h"
#include "Net/NetworkMetricsConfig.h"
#include "UnitTests/GorgeousObjectVariablePerfTestTypes.h"
#include "UnitTests/Harness/GorgeousHarnessGameInstance.h"
#include "UnitTests/Harness/GorgeousHarnessGameMode.h"
#include "UObject/StrongObjectPtr.h"

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousHarness, Log, All);

FGorgeousServerClientHarness::FGorgeousServerClientHarness()
{
}

FGorgeousServerClientHarness::~FGorgeousServerClientHarness()
{
	Shutdown();
}

FString FGorgeousServerClientHarness::FDriverStatSnapshot::Describe() const
{
	if (Label.IsEmpty())
	{
		return FString();
	}

	if (!bValid)
	{
		return FString::Printf(TEXT("%s stat net unavailable (driver offline)"), *Label);
	}

	const double InRateKBs = static_cast<double>(InRateBytesPerSecond) / 1024.0;
	const double OutRateKBs = static_cast<double>(OutRateBytesPerSecond) / 1024.0;
	const TCHAR* IrisState = bIsUsingIris ? TEXT("true") : TEXT("false");

	TArray<FString> Lines;
	Lines.Add(FString::Printf(TEXT("%s stat net"), *Label));
	Lines.Add(FString::Printf(TEXT("  Rate: In=%.1f KB/s Out=%.1f KB/s"), InRateKBs, OutRateKBs));

	if (!bHasConnection)
	{
		Lines.Add(FString::Printf(TEXT("  Iris=%s | Connection=<none>"), IrisState));
		return FString::Join(Lines, TEXT("\n"));
	}

	const FString PacketLine = FString::Printf(TEXT("  Packets: In=%llu (%.0f/s) Out=%llu (%.0f/s)"),
		static_cast<unsigned long long>(InPackets),
		static_cast<double>(InPacketsPerSecond),
		static_cast<unsigned long long>(OutPackets),
		static_cast<double>(OutPacketsPerSecond));
	Lines.Add(PacketLine);
	Lines.Add(FString::Printf(TEXT("  Loss: In=%.2f%%%% Out=%.2f%%%%"), InLossPercent, OutLossPercent));
	Lines.Add(FString::Printf(TEXT("  Latency: Ping=%.2f ms | Jitter=%.2f ms"), AvgPingMs, AvgJitterMs));
	Lines.Add(FString::Printf(TEXT("  Iris=%s | ConnState=%s"), IrisState, LexToString(ConnectionState)));
	return FString::Join(Lines, TEXT("\n"));
}

#if !WITH_SERVER_CODE

bool FGorgeousServerClientHarness::Initialize(const FGorgeousServerClientHarnessOptions& InOptions, FString& OutError)
{
	Options = InOptions;
	OutError = TEXT("Server/client harness unavailable in this build.");
	return false;
}

void FGorgeousServerClientHarness::Tick(float, int32)
{
}

void FGorgeousServerClientHarness::Shutdown()
{
}

bool FGorgeousServerClientHarness::HasLiveConnection() const
{
	return false;
}

UWorld* FGorgeousServerClientHarness::GetServerWorld() const
{
	return nullptr;
}

UWorld* FGorgeousServerClientHarness::GetClientWorld() const
{
	return nullptr;
}

FGorgeousServerClientHarness::FCollectedStats FGorgeousServerClientHarness::GatherStats() const
{
	return {};
}

void FGorgeousServerClientHarness::CollectRegisteredNetworkMetrics(TArray<FString>&) const
{
}
#else // WITH_SERVER_CODE

#include "Engine/NetworkSettings.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "HAL/PlatformTime.h"
#include "Misc/Paths.h"
#include "Net/UnrealNetwork.h"

namespace
{
	static constexpr int32 GPerfHarnessPortMin = 42000;
	static constexpr int32 GPerfHarnessPortMax = 45000;
	static FCriticalSection GHarnessPortMutex;

		// Create a local player even when no GameViewportClient exists (headless automation workers).
		static ULocalPlayer* CreateHeadlessLocalPlayer(UGameInstance* GameInstance, FString& OutError)
		{
			if (!GameInstance)
			{
				OutError = TEXT("Invalid game instance for headless local player creation.");
				return nullptr;
			}

			const int32 MaxSplitscreenPlayers = 1;
			FPlatformUserId UserId = FGenericPlatformMisc::GetPlatformUserForUserIndex(0);
			if (!UserId.IsValid())
			{
				UserId = FPlatformUserId::CreateFromInternalId(0);
			}

			// ULocalPlayer's ClassWithin is UGameEngine, so we need the engine as the outer to satisfy UObject's within contract.
			UObject* PlayerOuter = GameInstance->GetEngine() ? static_cast<UObject*>(GameInstance->GetEngine()) : static_cast<UObject*>(GEngine);
			ULocalPlayer* NewPlayer = NewObject<ULocalPlayer>(PlayerOuter, GameInstance->GetEngine()->LocalPlayerClass);
			if (!NewPlayer)
			{
				OutError = TEXT("Failed to allocate headless local player.");
				return nullptr;
			}

			const int32 InsertIndex = GameInstance->AddLocalPlayer(NewPlayer, UserId);
			if (InsertIndex == INDEX_NONE)
			{
				OutError = FString::Printf(TEXT("Failed to add headless local player (MaxPlayers=%d)."), MaxSplitscreenPlayers);
				return nullptr;
			}

			return NewPlayer;
		}

	ENetworkMetricEnableMode ParseMetricEnableMode(const FString& ModeString)
	{
		if (ModeString.Equals(TEXT("EnableForIrisOnly"), ESearchCase::IgnoreCase))
		{
			return ENetworkMetricEnableMode::EnableForIrisOnly;
		}
		if (ModeString.Equals(TEXT("EnableForNonIrisOnly"), ESearchCase::IgnoreCase))
		{
			return ENetworkMetricEnableMode::EnableForNonIrisOnly;
		}
		return ENetworkMetricEnableMode::EnableForAllReplication;
	}

	bool ParseNetworkMetricConfigEntry(const FString& RawEntry, FNetworkMetricConfig& OutConfig)
	{
		FString WorkingEntry = RawEntry;
		WorkingEntry.TrimStartAndEndInline();
		if (WorkingEntry.StartsWith(TEXT("(")) && WorkingEntry.EndsWith(TEXT(")")) && WorkingEntry.Len() >= 2)
		{
			WorkingEntry = WorkingEntry.Mid(1, WorkingEntry.Len() - 2);
		}

		TMap<FString, FString> KeyValuePairs;
		TArray<FString> Tokens;
		WorkingEntry.ParseIntoArray(Tokens, TEXT(","), true);
		for (FString Token : Tokens)
		{
			Token.TrimStartAndEndInline();
			FString Key;
			FString Value;
			if (!Token.Split(TEXT("="), &Key, &Value))
			{
				continue;
			}

			Key.TrimStartAndEndInline();
			Key = Key.ToLower();
			Value.TrimStartAndEndInline();
			if (!KeyValuePairs.Contains(Key))
			{
				KeyValuePairs.Add(Key, Value);
			}
		}

		const FString* MetricNameStr = KeyValuePairs.Find(TEXT("metricname"));
		if (!MetricNameStr || MetricNameStr->IsEmpty())
		{
			return false;
		}

		OutConfig = FNetworkMetricConfig();
		OutConfig.MetricName = FName(**MetricNameStr);

		const FString* ClassNameStr = KeyValuePairs.Find(TEXT("classname"));
		const FString* ClassStr = ClassNameStr ? ClassNameStr : KeyValuePairs.Find(TEXT("class"));
		if (ClassStr && !ClassStr->IsEmpty())
		{
			OutConfig.Class = TSoftClassPtr<UNetworkMetricsBaseListener>(FSoftObjectPath(**ClassStr));
		}

		if (const FString* EnableModeStr = KeyValuePairs.Find(TEXT("enablemode")))
		{
			OutConfig.EnableMode = ParseMetricEnableMode(*EnableModeStr);
		}

		return true;
	}
}

void FGorgeousServerClientHarness::FPerfNetEndpoint::Reset()
{
	NetDriver = nullptr;
	NetDriverName = NAME_None;
	FGorgeousServerClientHarness::ClearEndpointPendingNetGame(*this);
	HarnessGameInstance.Reset();
	MetricsCollector.Reset();
	Scope.Destroy();
	StatBaseline = {};
	RegisteredMetricConfigs.Reset();
	bMetricsCollectorRegistered = false;
}

bool FGorgeousServerClientHarness::Initialize(const FGorgeousServerClientHarnessOptions& InOptions, FString& OutError)
{
	Options = InOptions;

	if (!BootstrapServer(OutError))
	{
		Shutdown();
		return false;
	}

	if (!BootstrapClient(OutError))
	{
		Shutdown();
		return false;
	}

	if (!WaitForHandshake(Options.ConnectTimeoutSeconds, Options.TickStepSeconds, OutError))
	{
		Shutdown();
		return false;
	}

	// After the handshake, the server spawns a player controller for the client connection.
	// We need to wait for this to be replicated to the client world.
	if (!WaitForPlayerControllers(Options.ConnectTimeoutSeconds, Options.TickStepSeconds, OutError))
	{
		Shutdown();
		return false;
	}

	ForceEndpointConnectionsOpen(Server);
	ForceEndpointConnectionsOpen(Client);

	if (Client.NetDriver && Client.Scope.World)
	{
		Client.NetDriver->SetWorld(Client.Scope.World);
		Client.Scope.World->SetNetDriver(Client.NetDriver);
	}
	ClearEndpointPendingNetGame(Client);
	EnableStatCollection();

	bInitialized = true;
	UE_LOG(LogGorgeousHarness, Log, TEXT("[Harness] Connected server/client on %s:%d"), *Options.ListenAddress, ServerPort);
	return true;
}

void FGorgeousServerClientHarness::Tick(float DeltaSeconds, int32 Iterations)
{
	if (!bInitialized)
	{
		return;
	}

	Iterations = FMath::Max(Iterations, 1);
	for (int32 Index = 0; Index < Iterations; ++Index)
	{
		PumpOnce(DeltaSeconds);
	}
}

void FGorgeousServerClientHarness::Shutdown()
{
	// Clean up the GorgeousRootObjectVariable registry before destroying worlds.
	// The registry may hold references to objects (like GorgeousPlayerState, GorgeousGameState)
	// that exist in the harness worlds. If we don't clean these up first, the engine will
	// detect "world leaks" because the registry's strong references prevent GC from collecting
	// the old world objects.
	UGorgeousRootObjectVariable::CleanupRegistry(true);

	if (Server.NetDriver && GEngine && Server.Scope.World)
	{
		GEngine->DestroyNamedNetDriver(Server.Scope.World, Server.NetDriverName.IsNone() ? NAME_GameNetDriver : Server.NetDriverName);
		Server.NetDriver = nullptr;
		Server.NetDriverName = NAME_None;
	}

	if (Client.NetDriver && GEngine && Client.Scope.World)
	{
		GEngine->DestroyNamedNetDriver(Client.Scope.World, Client.NetDriverName.IsNone() ? NAME_GameNetDriver : Client.NetDriverName);
		Client.NetDriver = nullptr;
		Client.NetDriverName = NAME_None;
	}

	ClearEndpointPendingNetGame(Client);

	Server.Scope.Destroy();
	Client.Scope.Destroy();
	ServerPort = 0;
	bInitialized = false;
	bStatsCollectionEnabled = false;
	StatsEnableTimeSeconds = 0.0;
	Server.StatBaseline = {};
	Client.StatBaseline = {};
}

bool FGorgeousServerClientHarness::HasLiveConnection() const
{
	const bool bClientStateValid = Client.NetDriver && Client.NetDriver->ServerConnection;
	const EConnectionState ClientState = bClientStateValid
		? Client.NetDriver->ServerConnection->GetConnectionState()
		: USOCK_Invalid;
	const bool bClientReady = bClientStateValid
		&& (ClientState == USOCK_Open || (ClientState == USOCK_Pending && !Client.PendingNetGame.IsValid()));
	const bool bServerReady = Server.NetDriver
		&& Server.NetDriver->ClientConnections.Num() > 0
		&& Server.NetDriver->ClientConnections[0]
		&& Server.NetDriver->ClientConnections[0]->GetConnectionState() == USOCK_Open;
	return bClientReady && bServerReady;
}

UWorld* FGorgeousServerClientHarness::GetServerWorld() const
{
	return Server.Scope.World.Get();
}

UWorld* FGorgeousServerClientHarness::GetClientWorld() const
{
	return Client.Scope.World.Get();
}

bool FGorgeousServerClientHarness::BootstrapServer(FString& OutError)
{
	if (!Server.Scope.Create(TEXT("GTHarnessServer"), EWorldType::Game, Options.HarnessMapPath, OutError))
	{
		return false;
	}

	if (Server.Scope.WorldContext)
	{
		Server.Scope.WorldContext->RunAsDedicated = true;
	}

	EnsureHarnessGameInstance(Server, TEXT("Server"));

	auto EnsureHarnessGameMode = [&]() -> bool
	{
		UWorld* ServerWorld = Server.Scope.World;
		if (!ServerWorld)
		{
			return false;
		}

		auto ForcePlayerControllerClass = [](AGameModeBase* GameMode)
		{
			if (!GameMode)
			{
				return;
			}

			// Tests expect both endpoints to use AGorgeousPlayerController; enforce even if the map/blueprint overrides.
			GameMode->PlayerControllerClass = AGorgeousPlayerController::StaticClass();
			GameMode->ReplaySpectatorPlayerControllerClass = AGorgeousPlayerController::StaticClass();
		};

		auto TryApplyHarnessGameMode = [&]() -> bool
		{
			if (AWorldSettings* WorldSettings = ServerWorld->GetWorldSettings())
			{
				WorldSettings->DefaultGameMode = AGorgeousHarnessGameMode::StaticClass();
			}

			const UPackage* WorldPackage = ServerWorld->PersistentLevel ? ServerWorld->PersistentLevel->GetOutermost() : ServerWorld->GetOutermost();
			const FString HarnessMapName = WorldPackage ? WorldPackage->GetName() : ServerWorld->GetMapName();
			const TCHAR* HarnessMap = HarnessMapName.IsEmpty() ? TEXT("") : *HarnessMapName;
			FURL HarnessURL(nullptr, HarnessMap, TRAVEL_Absolute);
			const FString GameOption = FString::Printf(TEXT("Game=%s"), *AGorgeousHarnessGameMode::StaticClass()->GetPathName());
			HarnessURL.AddOption(*GameOption);
			if (!ServerWorld->SetGameMode(HarnessURL))
			{
				return false;
			}

			if (AGameModeBase* GameMode = ServerWorld->GetAuthGameMode())
			{
				ForcePlayerControllerClass(GameMode);
				return true;
			}

			return false;
		};

		AGameModeBase* ExistingGameMode = ServerWorld->GetAuthGameMode();
		if (ExistingGameMode)
		{
			if (!ExistingGameMode->IsA(AGorgeousHarnessGameMode::StaticClass()))
			{
				UE_LOG(LogGorgeousHarness, Warning, TEXT("[Harness] Server GameMode was %s instead of %s; forcing harness mode."),
					*ExistingGameMode->GetClass()->GetName(), *AGorgeousHarnessGameMode::StaticClass()->GetName());
				if (!TryApplyHarnessGameMode())
				{
					return false;
				}
			}

			ForcePlayerControllerClass(ServerWorld->GetAuthGameMode());
			return true;
		}

		return TryApplyHarnessGameMode();
	};

	if (!EnsureHarnessGameMode())
	{
		Server.Scope.Destroy();
		OutError = TEXT("Failed to initialize harness GameMode.");
		return false;
	}

	const int32 MaxAttempts = 8;
	FString LastError;
	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		const int32 CandidatePort = AllocateListenPort();
		const FName ServerDriverName(*FString::Printf(TEXT("GTHarnessServer_%d_%d"), FPlatformTLS::GetCurrentThreadId(), Attempt));
		if (!GEngine->CreateNamedNetDriver(Server.Scope.World, ServerDriverName, NAME_GameNetDriver))
		{
			LastError = TEXT("Failed to create server net driver instance.");
			continue;
		}

		UNetDriver* NewServerDriver = GEngine->FindNamedNetDriver(Server.Scope.World, ServerDriverName);
		if (!NewServerDriver)
		{
			LastError = TEXT("Server net driver lookup failed.");
			GEngine->DestroyNamedNetDriver(Server.Scope.World, ServerDriverName);
			continue;
		}

		NewServerDriver->SetWorld(Server.Scope.World);
		Server.Scope.World->SetNetDriver(NewServerDriver);

		FURL ListenURL(nullptr, *Options.ListenAddress, TRAVEL_Absolute);
		ListenURL.Port = CandidatePort;
		FString ListenError;
		if (!NewServerDriver->InitListen(Server.Scope.World, ListenURL, true, ListenError))
		{
			LastError = ListenError;
			Server.Scope.World->SetNetDriver(nullptr);
			GEngine->DestroyNamedNetDriver(Server.Scope.World, ServerDriverName);
			continue;
		}

		Server.NetDriver = NewServerDriver;
		Server.NetDriverName = ServerDriverName;
		ServerPort = CandidatePort;
		FString LocalAddrString = Server.NetDriver->GetLocalAddr().IsValid() ? Server.NetDriver->GetLocalAddr()->ToString(true) : TEXT("<unknown>");
		UE_LOG(LogGorgeousHarness, Log, TEXT("[Harness] Server listening on %s:%d (LocalAddr=%s Driver=%s)"),
			*Options.ListenAddress, ServerPort, *LocalAddrString, *Server.NetDriver->GetClass()->GetName());
		return true;
	}

	Server.Scope.Destroy();
	OutError = LastError.IsEmpty() ? TEXT("Unable to bind harness server port.") : LastError;
	return false;
}

bool FGorgeousServerClientHarness::BootstrapClient(FString& OutError)
{
	if (!Client.Scope.Create(TEXT("GTHarnessClient"), EWorldType::Game, Options.HarnessMapPath, OutError))
	{
		return false;
	}

	// Ensure the client world/context is not flagged as dedicated; automation runs under -server sometimes.
	if (Client.Scope.WorldContext)
	{
		Client.Scope.WorldContext->RunAsDedicated = false;
	}
	EnsureHarnessGameInstance(Client, TEXT("Client"));

	// Ensure the client world has at least one local player so the PendingNetGame handshake
	// can issue a join request and the server will spawn a player controller for us.
	if (UGameInstance* ClientGameInstance = Client.Scope.World ? Client.Scope.World->GetGameInstance() : nullptr)
	{
		if (ClientGameInstance->IsDedicatedServerInstance())
		{
			OutError = TEXT("Harness client world is running as a dedicated server; cannot create a local player.");
			Client.Scope.Destroy();
			return false;
		}

		if (!ClientGameInstance->FindLocalPlayerFromControllerId(0))
		{
			FString LocalPlayerError;
			ULocalPlayer* LocalPlayer = nullptr;
			if (ClientGameInstance->GetGameViewportClient())
			{
				LocalPlayer = ClientGameInstance->CreateLocalPlayer(0, LocalPlayerError, false);
			}
			else
			{
				LocalPlayer = CreateHeadlessLocalPlayer(ClientGameInstance, LocalPlayerError);
			}
			if (!LocalPlayer)
			{
				OutError = LocalPlayerError.IsEmpty() ? TEXT("Failed to create client local player.") : LocalPlayerError;
				Client.Scope.Destroy();
				return false;
			}
		}

		if (ClientGameInstance->GetLocalPlayers().Num() == 0)
		{
			OutError = TEXT("Client game instance has no local players after creation.");
			Client.Scope.Destroy();
			return false;
		}
	}

	// Set up the URL to use the harness map. This is critical because the join request
	// uses WorldContext->LastURL to tell the server which map the client wants to join.
	// Without this, it would fall back to GameDefaultMap (e.g., MainMenu_Map).
	FURL HarnessMapURL(nullptr, *Options.HarnessMapPath, TRAVEL_Absolute);
	if (Client.Scope.WorldContext)
	{
		Client.Scope.WorldContext->LastURL = HarnessMapURL;
		Client.Scope.WorldContext->TravelURL = HarnessMapURL.ToString();
	}

	FURL ConnectURL(nullptr, *Options.ListenAddress, TRAVEL_Absolute);
	ConnectURL.Host = Options.ListenAddress;
	ConnectURL.Port = ServerPort;
	// Use the dedicated harness map asset so server/client stay in sync (avoids falling back to GameDefaultMap).
	ConnectURL.Map = Options.HarnessMapPath;

	ClearEndpointPendingNetGame(Client);
	UPendingNetGame* PendingNetGame = NewObject<UPendingNetGame>(GetTransientPackage());
	if (!PendingNetGame)
	{
		OutError = TEXT("Failed to allocate pending net game instance.");
		Client.Scope.Destroy();
		return false;
	}
	PendingNetGame->Initialize(ConnectURL);
	Client.PendingNetGame = TStrongObjectPtr<UPendingNetGame>(PendingNetGame);
	if (Client.Scope.WorldContext)
	{
		Client.Scope.WorldContext->PendingNetGame = PendingNetGame;
	}

	const FName ClientDriverName(*FString::Printf(TEXT("GTHarnessClient_%d"), FPlatformTLS::GetCurrentThreadId()));
	if (!GEngine->CreateNamedNetDriver(Client.Scope.World, ClientDriverName, NAME_GameNetDriver))
	{
		OutError = TEXT("Failed to create client net driver instance.");
		ClearEndpointPendingNetGame(Client);
		Client.Scope.Destroy();
		return false;
	}

	Client.NetDriver = GEngine->FindNamedNetDriver(Client.Scope.World, ClientDriverName);
	if (!Client.NetDriver)
	{
		OutError = TEXT("Client net driver lookup failed.");
		GEngine->DestroyNamedNetDriver(Client.Scope.World, ClientDriverName);
		ClearEndpointPendingNetGame(Client);
		Client.Scope.Destroy();
		return false;
	}

	Client.NetDriverName = ClientDriverName;

	FString ConnectError;
	if (!Client.NetDriver->InitConnect(PendingNetGame, ConnectURL, ConnectError))
	{
		OutError = FString::Printf(TEXT("InitConnect failed: %s"), *ConnectError);
		GEngine->DestroyNamedNetDriver(Client.Scope.World, ClientDriverName);
		Client.NetDriver = nullptr;
		ClearEndpointPendingNetGame(Client);
		Client.Scope.Destroy();
		return false;
	}

	PendingNetGame->NetDriver = Client.NetDriver;
	PendingNetGame->BeginHandshake();

	UE_LOG(LogGorgeousHarness, Log, TEXT("[Harness] Client dialing %s:%d"), *ConnectURL.Host, ConnectURL.Port);

	return true;
}

bool FGorgeousServerClientHarness::WaitForHandshake(double TimeoutSeconds, double TickStepSeconds, FString& OutError)
{
	const double StartTime = FPlatformTime::Seconds();
	const double Deadline = StartTime + TimeoutSeconds;
	const float Step = static_cast<float>(TickStepSeconds);

	auto MaybeSendClientJoin = [&]()
	{
		if (!Client.PendingNetGame.IsValid() || !Client.NetDriver || !Client.NetDriver->ServerConnection)
		{
			return;
		}

		UPendingNetGame* PendingNetGame = Client.PendingNetGame.Get();
		if (!PendingNetGame->bSuccessfullyConnected || PendingNetGame->bSentJoinRequest)
		{
			return;
		}

		if (CompletePendingClientTravel(Client))
		{
			UE_LOG(LogGorgeousHarness, Log, TEXT("[Harness] Client sending join request after handshake ack"));
		}
	};

	while (FPlatformTime::Seconds() < Deadline)
	{
		PumpOnce(Step);
		MaybeSendClientJoin();
		if (HasLiveConnection())
		{
			// Reinforce engine-level GameInstance pointer after travel so subsystems (e.g., CommonUI) find a valid instance.
			if (UGameEngine* Engine = Cast<UGameEngine>(GEngine))
			{
				if (Client.Scope.World && Client.Scope.World->GetGameInstance())
				{
					Engine->GameInstance = Client.Scope.World->GetGameInstance();
				}
			}

			if (!bLoggedConnectionSuccess)
			{
				const FString ServerWorldName = Server.Scope.World ? (Server.Scope.World->GetOutermost() ? Server.Scope.World->GetOutermost()->GetName() : Server.Scope.World->GetMapName()) : TEXT("<null>");
				const FString ClientWorldName = Client.Scope.World ? (Client.Scope.World->GetOutermost() ? Client.Scope.World->GetOutermost()->GetName() : Client.Scope.World->GetMapName()) : TEXT("<null>");
				UE_LOG(LogGorgeousHarness, Log, TEXT("[Harness] Connection established (ServerWorld=%s ClientWorld=%s)."), *ServerWorldName, *ClientWorldName);
				bLoggedConnectionSuccess = true;
			}
			return true;
		}
		FPlatformProcess::SleepNoStats(TickStepSeconds);
	}

	const double ElapsedSeconds = FPlatformTime::Seconds() - StartTime;
	const int32 PumpIterations = static_cast<int32>(ElapsedSeconds / TickStepSeconds);
	const EConnectionState ClientState = (Client.NetDriver && Client.NetDriver->ServerConnection)
		? Client.NetDriver->ServerConnection->GetConnectionState()
		: USOCK_Invalid;
	const bool bServerHasConnection = Server.NetDriver && Server.NetDriver->ClientConnections.Num() > 0 && Server.NetDriver->ClientConnections[0];
	const EConnectionState ServerState = bServerHasConnection
		? Server.NetDriver->ClientConnections[0]->GetConnectionState()
		: USOCK_Invalid;
	const int32 ServerConnCount = Server.NetDriver ? Server.NetDriver->ClientConnections.Num() : 0;
	OutError = FString::Printf(TEXT("Harness handshake timed out after %.2fs (%d pumps) (ClientState=%s, ServerState=%s, ServerConnCount=%d)."),
		ElapsedSeconds, PumpIterations,
		LexToString(ClientState), LexToString(ServerState), ServerConnCount);
	return false;
}

bool FGorgeousServerClientHarness::WaitForPlayerControllers(double TimeoutSeconds, double TickStepSeconds, FString& OutError)
{
	const double StartTime = FPlatformTime::Seconds();
	const double Deadline = StartTime + TimeoutSeconds;
	const float Step = static_cast<float>(TickStepSeconds);

	auto HasServerPlayerController = [&]() -> bool
	{
		if (!Server.Scope.World)
		{
			return false;
		}
		for (FConstPlayerControllerIterator It = Server.Scope.World->GetPlayerControllerIterator(); It; ++It)
		{
			if (It->Get() && It->Get()->IsA(AGorgeousPlayerController::StaticClass()))
			{
				return true;
			}
		}
		return false;
	};

	auto HasClientPlayerController = [&]() -> bool
	{
		if (!Client.Scope.World)
		{
			return false;
		}
		for (FConstPlayerControllerIterator It = Client.Scope.World->GetPlayerControllerIterator(); It; ++It)
		{
			if (It->Get() && It->Get()->IsA(AGorgeousPlayerController::StaticClass()))
			{
				return true;
			}
		}
		return false;
	};

	// Helper to tick both net drivers and worlds - needed for player controller replication.
	// Unlike PumpOnce during handshake phase, we need to tick the worlds here so that
	// the PlayerController can be replicated from server to client.
	auto PumpWithWorldTick = [&]()
	{
		// Tick net drivers
		if (Server.NetDriver)
		{
			if (Server.Scope.World && Server.Scope.World->GetNetDriver() != Server.NetDriver)
			{
				Server.Scope.World->SetNetDriver(Server.NetDriver);
			}
			Server.NetDriver->TickDispatch(Step);
			Server.NetDriver->PostTickDispatch();
			Server.NetDriver->TickFlush(Step);
			Server.NetDriver->PostTickFlush();
		}

		if (Client.NetDriver)
		{
			if (Client.Scope.World && Client.Scope.World->GetNetDriver() != Client.NetDriver)
			{
				Client.Scope.World->SetNetDriver(Client.NetDriver);
			}
			Client.NetDriver->TickDispatch(Step);
			Client.NetDriver->PostTickDispatch();
			Client.NetDriver->TickFlush(Step);
			Client.NetDriver->PostTickFlush();
		}

		// Tick worlds - this is critical for actor replication to process
		if (Server.Scope.World)
		{
			Server.Scope.Tick(Step);
		}
		if (Client.Scope.World)
		{
			Client.Scope.Tick(Step);
		}
	};

	while (FPlatformTime::Seconds() < Deadline)
	{
		PumpWithWorldTick();

		if (HasServerPlayerController() && HasClientPlayerController())
		{
			UE_LOG(LogGorgeousHarness, Log, TEXT("[Harness] Player controllers ready on both endpoints."));
			return true;
		}

		FPlatformProcess::SleepNoStats(TickStepSeconds);
	}

	const double ElapsedSeconds = FPlatformTime::Seconds() - StartTime;
	const bool bHasServerPC = HasServerPlayerController();
	const bool bHasClientPC = HasClientPlayerController();
	OutError = FString::Printf(TEXT("Harness player controller wait timed out after %.2fs (ServerPC=%s, ClientPC=%s)."),
		ElapsedSeconds, bHasServerPC ? TEXT("true") : TEXT("false"), bHasClientPC ? TEXT("true") : TEXT("false"));
	return false;
}

bool FGorgeousServerClientHarness::PumpOnce(float DeltaSeconds)
{
	bool bTicked = false;
	const bool bHandshakePhase = !bInitialized;

	auto EnsureWorldNetDriver = [](FPerfNetEndpoint& Endpoint)
	{
		if (Endpoint.Scope.World && Endpoint.NetDriver && Endpoint.Scope.World->GetNetDriver() != Endpoint.NetDriver)
		{
			Endpoint.Scope.World->SetNetDriver(Endpoint.NetDriver);
		}
	};

	auto TickWorld = [&](FPerfNetEndpoint& Endpoint)
	{
		if (!Endpoint.Scope.World)
		{
			return;
		}
		Endpoint.Scope.Tick(DeltaSeconds);
		bTicked = true;
	};

	auto TickNetDriver = [&](FPerfNetEndpoint& Endpoint)
	{
		if (!Endpoint.NetDriver)
		{
			return;
		}
		EnsureWorldNetDriver(Endpoint);
		Endpoint.NetDriver->TickDispatch(DeltaSeconds);
		Endpoint.NetDriver->PostTickDispatch();
		Endpoint.NetDriver->TickFlush(DeltaSeconds);
		Endpoint.NetDriver->PostTickFlush();
		bTicked = true;
	};

	if (bHandshakePhase)
	{
		EnsureWorldNetDriver(Server);
		TickNetDriver(Server);
		if (Client.PendingNetGame)
		{
			Client.PendingNetGame->Tick(DeltaSeconds);
			bTicked = true;
		}
		else
		{
			EnsureWorldNetDriver(Client);
			TickNetDriver(Client);
		}
	}
	else
	{
		EnsureWorldNetDriver(Server);
		EnsureWorldNetDriver(Client);
		TickNetDriver(Server);
		TickNetDriver(Client);
		TickWorld(Server);
		TickWorld(Client);
	}

	return bTicked;
}

int32 FGorgeousServerClientHarness::AllocateListenPort()
{
	FScopeLock Lock(&GHarnessPortMutex);
	static int32 CurrentPort = GPerfHarnessPortMin;
	int32 Port = CurrentPort++;
	if (CurrentPort >= GPerfHarnessPortMax)
	{
		CurrentPort = GPerfHarnessPortMin;
	}
	return Port;
}

FGorgeousServerClientHarness::FCollectedStats FGorgeousServerClientHarness::GatherStats() const
{
	FCollectedStats Stats;
	const double ElapsedSeconds = (StatsEnableTimeSeconds > 0.0) ? (FPlatformTime::Seconds() - StatsEnableTimeSeconds) : 0.0;
	Stats.ServerSnapshot = CaptureNetDriverSnapshot(TEXT("Harness server"), Server.NetDriver, &Server.StatBaseline, ElapsedSeconds);
	Stats.ClientSnapshot = CaptureNetDriverSnapshot(TEXT("Harness client"), Client.NetDriver, &Client.StatBaseline, ElapsedSeconds);
	Stats.OutgoingPackets = Stats.ServerSnapshot.OutPackets + Stats.ClientSnapshot.OutPackets;
	Stats.IncomingPackets = Stats.ServerSnapshot.InPackets + Stats.ClientSnapshot.InPackets;
	Stats.OutgoingBytes = Stats.ServerSnapshot.OutBytes + Stats.ClientSnapshot.OutBytes;
	Stats.IncomingBytes = Stats.ServerSnapshot.InBytes + Stats.ClientSnapshot.InBytes;
	return Stats;
}

bool FGorgeousServerClientHarness::ShouldEmitMetricForDriver(const UNetDriver& NetDriver, const FNetworkMetricConfig& Config)
{
	if (Config.EnableMode == ENetworkMetricEnableMode::EnableForAllReplication)
	{
		return true;
	}

	const bool bUsingIris = NetDriver.IsUsingIrisReplication();
	return (Config.EnableMode == ENetworkMetricEnableMode::EnableForIrisOnly && bUsingIris)
		|| (Config.EnableMode == ENetworkMetricEnableMode::EnableForNonIrisOnly && !bUsingIris);
}

bool FGorgeousServerClientHarness::LoadHarnessNetworkMetricConfigs(TArray<FNetworkMetricConfig>& OutConfigs)
{
	OutConfigs.Reset();
	if (!GConfig)
	{
		return false;
	}

	const FConfigSection* Section = GConfig->GetSection(TEXT("/Script/Engine.NetworkMetricsConfig"), false, GEngineIni);
	if (!Section)
	{
		return false;
	}

	const FName ListenersKey(TEXT("Listeners"));
	for (const auto& Pair : *Section)
	{
		if (Pair.Key != ListenersKey)
		{
			continue;
		}

		FNetworkMetricConfig ParsedConfig;
		if (ParseNetworkMetricConfigEntry(Pair.Value.GetValue(), ParsedConfig))
		{
			OutConfigs.Add(MoveTemp(ParsedConfig));
		}
	}

	return OutConfigs.Num() > 0;
}

void FGorgeousServerClientHarness::RegisterEndpointMetrics(FPerfNetEndpoint& Endpoint)
{
	if (!Endpoint.NetDriver)
	{
		return;
	}

	UNetworkMetricsDatabase* Metrics = Endpoint.NetDriver->GetMetrics();
	if (!Metrics)
	{
		return;
	}

	TArray<FNetworkMetricConfig> MetricConfigs;
	if (!LoadHarnessNetworkMetricConfigs(MetricConfigs))
	{
		return;
	}

	if (!Endpoint.MetricsCollector.IsValid())
	{
		if (UGorgeousPerfNetworkMetricsListener* NewListener = NewObject<UGorgeousPerfNetworkMetricsListener>(Endpoint.NetDriver))
		{
			Endpoint.MetricsCollector = TStrongObjectPtr<UGorgeousPerfNetworkMetricsListener>(NewListener);
		}
	}

	if (!Endpoint.MetricsCollector.IsValid())
	{
		return;
	}

	Endpoint.RegisteredMetricConfigs = MoveTemp(MetricConfigs);

	for (const FNetworkMetricConfig& MetricConfig : Endpoint.RegisteredMetricConfigs)
	{
		if (!Metrics->Contains(MetricConfig.MetricName))
		{
			continue;
		}

		if (!ShouldEmitMetricForDriver(*Endpoint.NetDriver, MetricConfig))
		{
			continue;
		}

		Metrics->Register(MetricConfig.MetricName, Endpoint.MetricsCollector.Get());
	}

	Endpoint.bMetricsCollectorRegistered = true;
}

void FGorgeousServerClientHarness::CollectRegisteredNetworkMetrics(TArray<FString>& OutLines) const
{
	bool bEmittedAnyMetricLine = false;
	bool bObservedNonZeroMetric = false;

	auto DescribeEndpoint = [&](const TCHAR* Label, const FPerfNetEndpoint& Endpoint)
	{
		if (!Endpoint.NetDriver || !Endpoint.MetricsCollector.IsValid() || Endpoint.RegisteredMetricConfigs.Num() == 0)
		{
			return;
		}

		UNetworkMetricsDatabase* Metrics = Endpoint.NetDriver->GetMetrics();
		if (!Metrics)
		{
			return;
		}

		Metrics->ProcessListeners();

		for (const FNetworkMetricConfig& MetricConfig : Endpoint.RegisteredMetricConfigs)
		{
			if (!Metrics->Contains(MetricConfig.MetricName))
			{
				continue;
			}

			if (!ShouldEmitMetricForDriver(*Endpoint.NetDriver, MetricConfig))
			{
				continue;
			}

			FString ValueString;
			int64 IntValue = 0;
			float FloatValue = 0.0f;
			bool bHasValue = false;
			bool bValueNonZero = false;
			if (Endpoint.MetricsCollector->TryGetIntMetric(MetricConfig.MetricName, IntValue))
			{
				ValueString = LexToString(IntValue);
				bHasValue = true;
				bValueNonZero = IntValue != 0;
			}
			else if (Endpoint.MetricsCollector->TryGetFloatMetric(MetricConfig.MetricName, FloatValue))
			{
				ValueString = FString::SanitizeFloat(FloatValue);
				bHasValue = true;
				bValueNonZero = !FMath::IsNearlyZero(FloatValue);
			}
			else
			{
				ValueString = TEXT("<unavailable>");
			}

			OutLines.Add(FString::Printf(TEXT("%s metric %s.%s = %s"),
				Label,
				*MetricConfig.Class.ToString(),
				*MetricConfig.MetricName.ToString(),
				*ValueString));
			bEmittedAnyMetricLine = true;
			if (bHasValue && bValueNonZero)
			{
				bObservedNonZeroMetric = true;
			}
		}
	};

	DescribeEndpoint(TEXT("Harness server"), Server);
	DescribeEndpoint(TEXT("Harness client"), Client);

	if (bEmittedAnyMetricLine && !bObservedNonZeroMetric)
	{
		OutLines.Add(TEXT("Harness network metrics listeners returned only zero values (CSV/perf counter metrics focus on actor replication)."));
	}
}

void FGorgeousServerClientHarness::EnableStatCollection()
{
	if (bStatsCollectionEnabled)
	{
		return;
	}

	auto EnableForEndpoint = [](FPerfNetEndpoint& Endpoint, const TCHAR* EndpointLabel)
	{
		if (!Endpoint.NetDriver)
		{
			return;
		}

		Endpoint.NetDriver->bCollectNetStats = true;
		Endpoint.NetDriver->bSkipLocalStats = false;
		Endpoint.NetDriver->ProfileStats = true;
		Endpoint.NetDriver->StatPeriod = FMath::Max(Endpoint.NetDriver->StatPeriod, 0.25f);
		Endpoint.NetDriver->StatUpdateTime = FPlatformTime::Seconds();
		UE_LOG(LogGorgeousHarness, Verbose, TEXT("[Harness] %s driver enabling automation stat capture (Driver=%s)"),
			EndpointLabel,
			*Endpoint.NetDriver->GetClass()->GetName());
		RegisterEndpointMetrics(Endpoint);
	};

	EnableForEndpoint(Server, TEXT("server"));
	EnableForEndpoint(Client, TEXT("client"));
	CaptureEndpointStatBaseline(Server);
	CaptureEndpointStatBaseline(Client);
	StatsEnableTimeSeconds = FPlatformTime::Seconds();
	UE_LOG(LogGorgeousHarness, Verbose, TEXT("[Harness] Issuing stat net capture"));
	bStatsCollectionEnabled = true;
}

void FGorgeousServerClientHarness::ForceEndpointConnectionsOpen(FPerfNetEndpoint& Endpoint)
{
	if (!Endpoint.NetDriver)
	{
		return;
	}

	if (Endpoint.NetDriver->ServerConnection)
	{
		Endpoint.NetDriver->ServerConnection->SetConnectionState(USOCK_Open);
	}

	for (UNetConnection* Connection : Endpoint.NetDriver->ClientConnections)
	{
		if (Connection)
		{
			Connection->SetConnectionState(USOCK_Open);
		}
	}
}

bool FGorgeousServerClientHarness::FAutomationWorldScope::Create(const FString& InLabel, EWorldType::Type InWorldType, const FString& MapPath, FString& OutError)
{
	if (!GEngine)
	{
		OutError = TEXT("GEngine is not initialized.");
		return false;
	}

	if (MapPath.IsEmpty())
	{
		OutError = TEXT("Harness map path was not provided.");
		return false;
	}

	Label = InLabel;
	const bool bForcePIEForEditor = GIsEditor && InWorldType == EWorldType::Game;
	const EWorldType::Type EffectiveWorldType = bForcePIEForEditor ? EWorldType::PIE : InWorldType;
	WorldContext = &GEngine->CreateNewWorldContext(EffectiveWorldType);
	// In some editor contexts CreateNewWorldContext can carry a recycled world pointer; ensure we start clean.
	if (UWorld* ExistingWorld = WorldContext->World())
	{
		if (ExistingWorld->GetNetDriver())
		{
			ExistingWorld->SetNetDriver(nullptr);
		}
		ExistingWorld->DestroyWorld(false);
		GEngine->DestroyWorldContext(ExistingWorld);
		WorldContext = &GEngine->CreateNewWorldContext(EffectiveWorldType);
	}
	const FString GuidSuffix = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	const FString ContextHandleString = FString::Printf(TEXT("GTHarness_%s_%s"), *InLabel, *GuidSuffix);
	WorldContext->ContextHandle = FName(*ContextHandleString);
	// Force duplication when the target map is already initialized in another context (Game world load path uses PIEInstance to branch).
	static int32 GHarnessPieInstanceCounter = 1;
	WorldContext->PIEInstance = GHarnessPieInstanceCounter++;

	if (!BootstrapGameInstance.IsValid())
	{
		UGorgeousHarnessGameInstance* NewInstance = GEngine
			? NewObject<UGorgeousHarnessGameInstance>(GEngine)
			: NewObject<UGorgeousHarnessGameInstance>(GetTransientPackage());
		if (NewInstance)
		{
			BootstrapGameInstance = TStrongObjectPtr<UGorgeousHarnessGameInstance>(NewInstance);
		}
	}

	if (!BootstrapGameInstance.IsValid())
	{
		OutError = TEXT("Failed to allocate harness bootstrap GameInstance.");
		Destroy();
		return false;
	}

	WorldContext->OwningGameInstance = BootstrapGameInstance.Get();

	// LoadMap asserts if the context already has an initialized world or a live net driver.
	if (UWorld* ExistingWorld = WorldContext->World())
	{
		if (ExistingWorld->GetNetDriver())
		{
			ExistingWorld->SetNetDriver(nullptr);
		}
		WorldContext->SetCurrentWorld(nullptr);
	}

	FURL MapURL(nullptr, *MapPath, TRAVEL_Absolute);
	MapURL.AddOption(*FString::Printf(TEXT("Game=%s"), *AGorgeousHarnessGameMode::StaticClass()->GetPathName()));
	WorldContext->TravelURL = MapURL.ToString();
	WorldContext->LastURL = MapURL;

	FString LoadError;
	if (!GEngine->LoadMap(*WorldContext, MapURL, nullptr, LoadError))
	{
		OutError = LoadError.IsEmpty()
			? FString::Printf(TEXT("Failed to load harness map '%s'."), *MapPath)
			: LoadError;
		Destroy();
		return false;
	}

	World = WorldContext->World();
	if (!World)
	{
		OutError = FString::Printf(TEXT("Harness map '%s' did not produce a world."), *MapPath);
		Destroy();
		return false;
	}

	// Automation harnesses rely on Gauntlet-style multi-instance worlds. When running in the editor we
	// impersonate a PIE load to guarantee each endpoint receives an independent world/package, then restore
	// the requested world type so runtime checks still see a game world.
	if (bForcePIEForEditor)
	{
		WorldContext->WorldType = InWorldType;
		World->WorldType = InWorldType;
	}

	World->SetGameInstance(BootstrapGameInstance.Get());
	BootstrapGameInstance->AttachWorldContext(WorldContext);
	BootstrapGameInstance->Init();

	return true;
}

void FGorgeousServerClientHarness::FAutomationWorldScope::Tick(float DeltaSeconds)
{
	if (World)
	{
		World->Tick(ELevelTick::LEVELTICK_All, DeltaSeconds);
	}
}

void FGorgeousServerClientHarness::FAutomationWorldScope::Destroy()
{
	if (!World)
	{
		// Even without a world, we may have a game instance that needs cleanup.
		if (BootstrapGameInstance.IsValid())
		{
			BootstrapGameInstance->Shutdown();
			BootstrapGameInstance.Reset();
		}
		WorldContext = nullptr;
		Label.Reset();
		return;
	}

	// Shutdown the GameInstance BEFORE destroying the world context to ensure
	// subsystems are properly deinitialized. This prevents crashes during GC
	// when the destructor tries to deinitialize already-invalid subsystems.
	if (BootstrapGameInstance.IsValid())
	{
		BootstrapGameInstance->Shutdown();
	}

	if (GEngine)
	{
		GEngine->DestroyWorldContext(World);
	}

	World->DestroyWorld(false);
	World = nullptr;
	WorldContext = nullptr;
	BootstrapGameInstance.Reset();
	Label.Reset();
}

void FGorgeousServerClientHarness::ClearEndpointPendingNetGame(FPerfNetEndpoint& Endpoint)
{
	if (Endpoint.Scope.WorldContext && Endpoint.Scope.WorldContext->PendingNetGame == Endpoint.PendingNetGame.Get())
	{
		Endpoint.Scope.WorldContext->PendingNetGame = nullptr;
	}
	Endpoint.PendingNetGame.Reset();
}

bool FGorgeousServerClientHarness::CompletePendingClientTravel(FPerfNetEndpoint& Endpoint)
{
	if (!GEngine)
	{
		return false;
	}

	if (!Endpoint.PendingNetGame.IsValid())
	{
		return false;
	}

	FWorldContext* WorldContext = Endpoint.Scope.WorldContext;
	if (!WorldContext)
	{
		return false;
	}

	UPendingNetGame* PendingNetGame = Endpoint.PendingNetGame.Get();
	if (WorldContext->PendingNetGame != PendingNetGame)
	{
		WorldContext->PendingNetGame = PendingNetGame;
	}

	if (!PendingNetGame->LoadMapCompleted(GEngine, *WorldContext, true, TEXT("")))
	{
		return false;
	}

	PendingNetGame->TravelCompleted(GEngine, *WorldContext);

	WorldContext->PendingNetGame = nullptr;
	Endpoint.PendingNetGame.Reset();
	return true;
}

void FGorgeousServerClientHarness::CaptureEndpointStatBaseline(FPerfNetEndpoint& Endpoint)
{
	if (!Endpoint.NetDriver)
	{
		Endpoint.StatBaseline = {};
		return;
	}

	Endpoint.StatBaseline.BaseInBytes = Endpoint.NetDriver->InBytes;
	Endpoint.StatBaseline.BaseOutBytes = Endpoint.NetDriver->OutBytes;
	Endpoint.StatBaseline.BaseInPackets = Endpoint.NetDriver->InPackets;
	Endpoint.StatBaseline.BaseOutPackets = Endpoint.NetDriver->OutPackets;
	Endpoint.StatBaseline.bValid = true;
}

void FGorgeousServerClientHarness::EnsureHarnessGameInstance(FPerfNetEndpoint& Endpoint, const TCHAR* DebugLabel)
{
	FWorldContext* WorldContext = Endpoint.Scope.WorldContext;
	UWorld* World = Endpoint.Scope.World;
	if (!WorldContext || !World)
	{
		return;
	}

	UGameInstance* TargetInstance = World->GetGameInstance();
	if (!TargetInstance)
	{
		if (!Endpoint.HarnessGameInstance.IsValid())
		{
			UGorgeousHarnessGameInstance* NewInstance = GEngine
				? NewObject<UGorgeousHarnessGameInstance>(GEngine)
				: NewObject<UGorgeousHarnessGameInstance>(GetTransientPackage());
			if (NewInstance)
			{
				NewInstance->AttachWorldContext(WorldContext);
				NewInstance->Init();
				Endpoint.HarnessGameInstance = TStrongObjectPtr<UGameInstance>(NewInstance);
			}
			else
			{
				UE_LOG(LogGorgeousHarness, Warning, TEXT("[Harness] %s endpoint failed to allocate GameInstance"), DebugLabel);
			}
		}

		TargetInstance = Endpoint.HarnessGameInstance.Get();
	}

	if (TargetInstance)
	{
		WorldContext->OwningGameInstance = TargetInstance;
		World->SetGameInstance(TargetInstance);

		// Ensure the engine knows about this transient GameInstance so subsystems expecting GEngine->GameInstance (e.g., CommonUI) can find it.
		if (UGameEngine* Engine = Cast<UGameEngine>(GEngine))
		{
			Engine->GameInstance = TargetInstance;
		}
	}
	else
	{
		UE_LOG(LogGorgeousHarness, Warning, TEXT("[Harness] %s endpoint missing GameInstance"), DebugLabel);
	}
}

FGorgeousServerClientHarness::FDriverStatSnapshot FGorgeousServerClientHarness::CaptureNetDriverSnapshot(const TCHAR* Label, UNetDriver* NetDriver, const FPerfNetEndpoint::FStatBaseline* Baseline, double ElapsedSeconds)
{
	FDriverStatSnapshot Snapshot;
	if (Label && *Label)
	{
		Snapshot.Label = Label;
	}

	if (!NetDriver)
	{
		return Snapshot;
	}

	Snapshot.bValid = true;
	Snapshot.InPackets = NetDriver->InPackets;
	Snapshot.OutPackets = NetDriver->OutPackets;
	Snapshot.InBytes = NetDriver->InBytes;
	Snapshot.OutBytes = NetDriver->OutBytes;
	Snapshot.InBunches = NetDriver->InBunches;
	Snapshot.OutBunches = NetDriver->OutBunches;
	Snapshot.InPacketsLost = NetDriver->InPacketsLost;
	Snapshot.OutPacketsLost = NetDriver->OutPacketsLost;
	Snapshot.ObservedSeconds = ElapsedSeconds;
	Snapshot.bIsUsingIris = NetDriver->IsUsingIrisReplication();

	const bool bUseBaseline = Baseline && Baseline->bValid && ElapsedSeconds > KINDA_SMALL_NUMBER;
	if (bUseBaseline)
	{
		auto ComputeRate = [ElapsedSeconds](double Delta)
		{
			return Delta <= 0.0 ? 0.0 : Delta / ElapsedSeconds;
		};

		const double DeltaInBytes = static_cast<double>(NetDriver->InBytes) - static_cast<double>(Baseline->BaseInBytes);
		const double DeltaOutBytes = static_cast<double>(NetDriver->OutBytes) - static_cast<double>(Baseline->BaseOutBytes);
		const double DeltaInPackets = static_cast<double>(NetDriver->InPackets) - static_cast<double>(Baseline->BaseInPackets);
		const double DeltaOutPackets = static_cast<double>(NetDriver->OutPackets) - static_cast<double>(Baseline->BaseOutPackets);
		Snapshot.InRateBytesPerSecond = ComputeRate(DeltaInBytes);
		Snapshot.OutRateBytesPerSecond = ComputeRate(DeltaOutBytes);
		Snapshot.InPacketsPerSecond = ComputeRate(DeltaInPackets);
		Snapshot.OutPacketsPerSecond = ComputeRate(DeltaOutPackets);
	}
	else
	{
		Snapshot.InRateBytesPerSecond = NetDriver->InBytesPerSecond;
		Snapshot.OutRateBytesPerSecond = NetDriver->OutBytesPerSecond;
	}

	UNetConnection* ActiveConnection = NetDriver->ServerConnection;
	if (!ActiveConnection && NetDriver->ClientConnections.Num() > 0)
	{
		ActiveConnection = NetDriver->ClientConnections[0];
	}

	if (ActiveConnection)
	{
		Snapshot.bHasConnection = true;
		Snapshot.ConnectionState = ActiveConnection->GetConnectionState();
		Snapshot.AvgPingMs = ActiveConnection->AvgLag * 1000.0;
		Snapshot.AvgJitterMs = ActiveConnection->GetAverageJitterInMS();
		Snapshot.InLossPercent = ActiveConnection->GetInLossPercentage().GetAvgLossPercentage() * 100.0f;
		Snapshot.OutLossPercent = ActiveConnection->GetOutLossPercentage().GetAvgLossPercentage() * 100.0f;
		if (!bUseBaseline)
		{
			Snapshot.InPacketsPerSecond = ActiveConnection->InPacketsPerSecond;
			Snapshot.OutPacketsPerSecond = ActiveConnection->OutPacketsPerSecond;
		}
	}

	return Snapshot;
}

#endif // WITH_SERVER_CODE
