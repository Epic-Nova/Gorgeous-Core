#pragma once

#include "CoreMinimal.h"
#include "Engine/NetConnection.h"
#include "TraceServices/Model/NetProfiler.h"
#include "UObject/StrongObjectPtr.h"

class UGorgeousPerfNetworkMetricsListener;
class UGorgeousHarnessGameInstance;
class UGameInstance;
class UNetDriver;
class UPendingNetGame;
class UWorld;

struct FNetworkMetricConfig;

/** Options that control how the Gorgeous automation harness spins up networking worlds. */
struct FGorgeousServerClientHarnessOptions
{
	/** Timeout while awaiting client/server handshake completion. */
	double ConnectTimeoutSeconds = 20.0;

	/** Tick step used while pumping harness worlds and net drivers. */
	double TickStepSeconds = 1.0 / 120.0;

	/** Bind address for the harness server. Defaults to loopback. */
	FString ListenAddress = TEXT("127.0.0.1");

	/** Map to load for both harness endpoints (must be a valid map package path). */
	FString HarnessMapPath = TEXT("/Game/Developers/NilsB/UnitTests/ServerClientHarness");
};

/** Lightweight dedicated-server + client harness that mirrors the OV perf test environment. */
class FGorgeousServerClientHarness
{
public:
	struct FDriverStatSnapshot
	{
		FString Label = TEXT("Harness endpoint");
		uint64 InPackets = 0;
		uint64 OutPackets = 0;
		double InBytes = 0.0;
		double OutBytes = 0.0;
		uint64 InBunches = 0;
		uint64 OutBunches = 0;
		uint64 InPacketsLost = 0;
		uint64 OutPacketsLost = 0;
		double InRateBytesPerSecond = 0.0;
		double OutRateBytesPerSecond = 0.0;
		double InPacketsPerSecond = 0.0;
		double OutPacketsPerSecond = 0.0;
		double ObservedSeconds = 0.0;
		double AvgPingMs = 0.0;
		float AvgJitterMs = 0.0f;
		float InLossPercent = 0.0f;
		float OutLossPercent = 0.0f;
		bool bIsUsingIris = false;
		bool bHasConnection = false;
		TEnumAsByte<EConnectionState> ConnectionState = USOCK_Invalid;
		bool bValid = false;

		FString Describe() const;
	};

	struct FCollectedStats
	{
		uint64 OutgoingPackets = 0;
		uint64 IncomingPackets = 0;
		double OutgoingBytes = 0.0;
		double IncomingBytes = 0.0;
		FDriverStatSnapshot ServerSnapshot;
		FDriverStatSnapshot ClientSnapshot;
	};

	FGorgeousServerClientHarness();
	~FGorgeousServerClientHarness();

	bool Initialize(const FGorgeousServerClientHarnessOptions& InOptions, FString& OutError);
	void Tick(float DeltaSeconds, int32 Iterations = 1);
	void Shutdown();
	bool IsActive() const { return bInitialized; }
	bool HasLiveConnection() const;
	int32 GetServerPort() const { return ServerPort; }
 	UWorld* GetServerWorld() const;
 	UWorld* GetClientWorld() const;

	FCollectedStats GatherStats() const;
	void CollectRegisteredNetworkMetrics(TArray<FString>& OutLines) const;

#if WITH_SERVER_CODE
	UNetDriver* GetServerDriver() const { return Server.NetDriver; }
	UNetDriver* GetClientDriver() const { return Client.NetDriver; }
#endif

private:
#if WITH_SERVER_CODE
	struct FAutomationWorldScope
	{
		bool Create(const FString& InLabel, EWorldType::Type InWorldType, const FString& MapPath, FString& OutError);
		void Tick(float DeltaSeconds);
		void Destroy();

		FWorldContext* WorldContext = nullptr;
		TObjectPtr<UWorld> World = nullptr;
		FString Label;
		TStrongObjectPtr<UGorgeousHarnessGameInstance> BootstrapGameInstance;
	};

	struct FPerfNetEndpoint
	{
		struct FStatBaseline
		{
			uint64 BaseInBytes = 0;
			uint64 BaseOutBytes = 0;
			uint64 BaseInPackets = 0;
			uint64 BaseOutPackets = 0;
			bool bValid = false;
		};

		void Reset();

		FAutomationWorldScope Scope;
		FName NetDriverName = NAME_None;
		UNetDriver* NetDriver = nullptr;
		TStrongObjectPtr<UPendingNetGame> PendingNetGame;
		TStrongObjectPtr<UGameInstance> HarnessGameInstance;
		TStrongObjectPtr<UGorgeousPerfNetworkMetricsListener> MetricsCollector;
		FStatBaseline StatBaseline;
		TArray<FNetworkMetricConfig> RegisteredMetricConfigs;
		bool bMetricsCollectorRegistered = false;
	};

	static void ClearEndpointPendingNetGame(FPerfNetEndpoint& Endpoint);
	static bool CompletePendingClientTravel(FPerfNetEndpoint& Endpoint);
	static void CaptureEndpointStatBaseline(FPerfNetEndpoint& Endpoint);
	static void EnsureHarnessGameInstance(FPerfNetEndpoint& Endpoint, const TCHAR* DebugLabel);
	static bool LoadHarnessNetworkMetricConfigs(TArray<FNetworkMetricConfig>& OutConfigs);
	static bool ShouldEmitMetricForDriver(const UNetDriver& NetDriver, const FNetworkMetricConfig& Config);
	static void RegisterEndpointMetrics(FPerfNetEndpoint& Endpoint);
	static void ForceEndpointConnectionsOpen(FPerfNetEndpoint& Endpoint);

	bool BootstrapServer(FString& OutError);
	bool BootstrapClient(FString& OutError);
	bool WaitForHandshake(double TimeoutSeconds, double TickStepSeconds, FString& OutError);
	bool WaitForPlayerControllers(double TimeoutSeconds, double TickStepSeconds, FString& OutError);
	bool PumpOnce(float DeltaSeconds);
	int32 AllocateListenPort();
	void EnableStatCollection();
	static FDriverStatSnapshot CaptureNetDriverSnapshot(const TCHAR* Label, UNetDriver* NetDriver, const FPerfNetEndpoint::FStatBaseline* Baseline, double ElapsedSeconds);

	FGorgeousServerClientHarnessOptions Options;
	FPerfNetEndpoint Server;
	FPerfNetEndpoint Client;
	int32 ServerPort = 0;
	bool bInitialized = false;
	bool bStatsCollectionEnabled = false;
	double StatsEnableTimeSeconds = 0.0;
	bool bLoggedConnectionSuccess = false;
#else
	FGorgeousServerClientHarnessOptions Options;
#endif // WITH_SERVER_CODE
};
