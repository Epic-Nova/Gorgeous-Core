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

#include "AutoReplication/GorgeousAutoReplicationCoordinator.h"

#include "AutoReplication/GorgeousAutoReplicationGraph.h"
#include "AutoReplication/GorgeousAutoReplicationIrisBackend.h"
#include "ModuleCore/GorgeousAutoReplicationSettings.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "Helpers/Macros/GorgeousLoggingHelperMacros.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "ReplicationGraph.h"
#include "Containers/Ticker.h"
#include "Algo/Sort.h"

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousAutoReplicationCoordinator, Log, All);

namespace {
    TMap<TWeakObjectPtr<UWorld>, TUniquePtr<FGorgeousAutoReplicationCoordinator>> WorldCoordinators;
}

FGorgeousAutoReplicationCoordinator::FGorgeousAutoReplicationCoordinator()
	: bIrisInitialized(false)
	, bReplicationGraphInitialized(false)
	, bWarnedReplicationGraphIrisConflict(false)
	, bIsInitializingReplicationGraph(false)
{
}

FGorgeousAutoReplicationCoordinator& FGorgeousAutoReplicationCoordinator::Get(UWorld* World)
{
    static FGorgeousAutoReplicationCoordinator EmptyCoordinator;
    if (!World) return EmptyCoordinator;
    for (auto It=WorldCoordinators.CreateIterator(); It; ++It) if (!It.Key().IsValid()) It.RemoveCurrent();
    auto& Coordinator = WorldCoordinators.FindOrAdd(World);
    if (!Coordinator) Coordinator.Reset(new FGorgeousAutoReplicationCoordinator());
    Coordinator->Initialize(World);
    return *Coordinator;
}

void FGorgeousAutoReplicationCoordinator::TearDownForWorld(UWorld* DyingWorld)
{
    if (!DyingWorld) return;
    if (auto* Coordinator = WorldCoordinators.Find(DyingWorld)) (*Coordinator)->TearDown();
    WorldCoordinators.Remove(DyingWorld);
}

// QoL override API: set/clear runtime overrides that take precedence over developer settings.
void FGorgeousAutoReplicationCoordinator::SetUseIrisOverride(bool bEnable)
{
    bUseIrisOverrideSet = true;
    bUseIrisOverrideValue = bEnable;
}

void FGorgeousAutoReplicationCoordinator::ClearUseIrisOverride()
{
    bUseIrisOverrideSet = false;
}

void FGorgeousAutoReplicationCoordinator::SetReplicationGraphEnabledOverride(bool bEnable)
{
    bReplicationGraphEnabledOverrideSet = true;
    bReplicationGraphEnabledOverrideValue = bEnable;
}

void FGorgeousAutoReplicationCoordinator::ClearReplicationGraphEnabledOverride()
{
    bReplicationGraphEnabledOverrideSet = false;
}

void FGorgeousAutoReplicationCoordinator::SetAutoReplicationGraphClassOverride(TSubclassOf<UReplicationGraph> InGraphClass)
{
    AutoReplicationGraphClassOverride = InGraphClass;
}

void FGorgeousAutoReplicationCoordinator::ClearAutoReplicationGraphClassOverride()
{
    AutoReplicationGraphClassOverride = nullptr;
}

void FGorgeousAutoReplicationCoordinator::Initialize(UWorld* World)
{
	if (!World)
	{
		return;
	}

	if (CachedWorld.Get() != World)
	{
		TearDown();
		CachedWorld = World;
	}

	if (!CachedWorld.IsValid())
	{
		CachedWorld = World;
	}

	if (!AutoReplicationGraph.IsValid())
	{
		bReplicationGraphInitialized = false;
	}

	if (!TickHandle.IsValid() && World->GetNetMode() != NM_Client)
	{
		TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FGorgeousAutoReplicationCoordinator::HandleTickerTick), 0.0f);
	}

	const UGorgeousAutoReplicationSettings* Settings = UGorgeousAutoReplicationSettings::Get();
	// Determine runtime desires for Iris and ReplicationGraph. Overrides (set via the QoL API)
	// take precedence over the configured developer settings.
	const bool bSettingsWantsIris = Settings ? Settings->bEnableIris : false;
	const bool bSettingsWantsReplicationGraph = Settings ? Settings->bEnableReplicationGraph : false;
	const bool bWantsIris = bUseIrisOverrideSet ? bUseIrisOverrideValue : bSettingsWantsIris;
	const bool bWantsReplicationGraph = bReplicationGraphEnabledOverrideSet ? bReplicationGraphEnabledOverrideValue : bSettingsWantsReplicationGraph;

	if (bWantsIris && !bIrisInitialized)
	{
		// TODO: Expose Iris transport initialization through developer settings/Blueprint nodes so designers can toggle at runtime.
		InitializeIris(World);
	}

	if (bWantsReplicationGraph && !bReplicationGraphInitialized)
	{
		// TODO: Allow configuring replication-graph class/backends from QoL mixins before init.
		InitializeReplicationGraph(World);
	}
}

void FGorgeousAutoReplicationCoordinator::ReinitializePreservingStreams(UWorld* World)
{
    auto SavedPending=MoveTemp(PendingPropertyChanges);
    TArray<TPair<TWeakObjectPtr<UGorgeousObjectVariable>,FGorgeousAutoReplicationStreamConfig>> Saved;
    for (const auto& Pair:ActiveStreams) Saved.Emplace(Pair.Value.Target,Pair.Value.Config);
    for (const auto& Pair:Saved) if (Pair.Key.IsValid()) UnregisterObjectVariable(Pair.Key.Get());
    TearDown(); Initialize(World);
    for (const auto& Pair:Saved) if (Pair.Key.IsValid()) RegisterObjectVariable(Pair.Key.Get(),Pair.Value);
    PendingPropertyChanges=MoveTemp(SavedPending);
}

void FGorgeousAutoReplicationCoordinator::TearDown()
{
	ActiveStreams.Empty();
    PendingPropertyChanges.Empty();
	AutoReplicationGraph = nullptr;
	CachedWorld = nullptr;
	bIrisInitialized = false;
	bReplicationGraphInitialized = false;
	bWarnedReplicationGraphIrisConflict = false;
#if GORGEOUSCORE_WITH_IRIS
	if (IrisBackend.IsValid())
	{
		IrisBackend->Reset();
	}
	IrisBackend.Reset();
#endif
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}
}

bool FGorgeousAutoReplicationCoordinator::IsIrisStreamActive(const UGorgeousObjectVariable* Variable) const
{
    const auto* Stream = Variable ? ActiveStreams.Find(FObjectKey(Variable)) : nullptr;
    return Stream && Stream->IsRegisteredWithIris();
}

void FGorgeousAutoReplicationCoordinator::RegisterObjectVariable(UGorgeousObjectVariable* Variable, const FGorgeousAutoReplicationStreamConfig& Config)
{
	if (!Variable)
	{
		return;
	}

	UWorld* World = Variable->GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return;
	}

	Initialize(World);

	const FObjectKey StreamKey(Variable);
    if (ActiveStreams.Contains(StreamKey)) UnregisterObjectVariable(Variable);
	FGorgeousAutoReplicationActiveStream* Stream = ActiveStreams.Find(StreamKey);
	if (!Stream)
	{
		Stream = &ActiveStreams.Add(StreamKey, FGorgeousAutoReplicationActiveStream(Variable, Config));
	}

	Stream->Target = Variable;
	Stream->Config = Config;
	Stream->SetRegisteredWithReplicationGraph(false);
	Stream->SetRegisteredWithIris(false);
	Stream->RefreshStreamGuid(Variable);
	Stream->RootNetworkChannel = Variable->ShouldUseRootNetworkStack() ? Variable->GetEffectiveNetworkChannel() : NAME_None;

    const UGorgeousAutoReplicationSettings* Settings=UGorgeousAutoReplicationSettings::Get();
    const bool bGraphEnabled=bReplicationGraphEnabledOverrideSet ? bReplicationGraphEnabledOverrideValue : Settings && Settings->bEnableReplicationGraph;
    const bool bIrisEnabled=bUseIrisOverrideSet ? bUseIrisOverrideValue : Settings && Settings->bEnableIris;
	const bool bUsesReplicationGraph = bGraphEnabled && FGorgeousAutoReplicationBackendTraits::UsesReplicationGraph(Config.Backend);
	const bool bUsesIris = bIrisEnabled && FGorgeousAutoReplicationBackendTraits::UsesIris(Config.Backend);

#if GORGEOUSCORE_WITH_REPLICATION_GRAPH
	if (bUsesReplicationGraph)
	{
		if (UGorgeousAutoReplicationGraph* Graph = Cast<UGorgeousAutoReplicationGraph>(GetOrCreateReplicationGraph(World)))
		{
			Graph->RegisterObjectVariable(Variable);
			Stream->SetRegisteredWithReplicationGraph(true);
		}
		else
		{
			GT_W_LOG("GT.AutoReplication.Coordinator.Graph.Missing", TEXT("AutoReplication stream on %s requested the replication graph backend but no graph instance is available."), *Variable->GetName());
		}
	}
#else
	if (bUsesReplicationGraph)
	{
		GT_W_LOG("GT.AutoReplication.Coordinator.Graph.Disabled", TEXT("AutoReplication stream on %s requested the replication graph backend but the module was built without replication graph support."), *Variable->GetName());
	}
#endif

#if GORGEOUSCORE_WITH_IRIS
	if (bUsesIris)
	{
		if (!bIrisInitialized)
		{
			InitializeIris(World);
		}

		if (IrisBackend.IsValid())
		{
			IrisBackend->RegisterStream(Variable, Config, Stream->GetStreamGuid());
			Stream->SetRegisteredWithIris(true);
            // Register before the actor's first metadata poll exports its OV pointer.
            Variable->TryPublishNativeIrisState();
		}
		else
		{
			GT_W_LOG("GT.AutoReplication.Coordinator.Iris.Missing", TEXT("AutoReplication stream on %s requested the Iris backend but no Iris backend instance is available."), *Variable->GetName());
		}
	}
#else
	if (bUsesIris)
	{
		GT_W_LOG("GT.AutoReplication.Coordinator.Iris.Disabled", TEXT("AutoReplication stream on %s requested the Iris backend but Iris is not enabled for this build."), *Variable->GetName());
	}
#endif
}

void FGorgeousAutoReplicationCoordinator::UnregisterObjectVariable(UGorgeousObjectVariable* Variable)
{
	if (!Variable)
	{
		return;
	}

	const FObjectKey StreamKey(Variable);
	FGorgeousAutoReplicationStreamConfig PreviousConfig;
	bool bWasRegisteredWithGraph = false;
	bool bWasRegisteredWithIris = false;
	if (FGorgeousAutoReplicationActiveStream* ExistingStream = ActiveStreams.Find(StreamKey))
	{
		PreviousConfig = ExistingStream->Config;
		bWasRegisteredWithGraph = ExistingStream->IsRegisteredWithReplicationGraph();
		bWasRegisteredWithIris = ExistingStream->IsRegisteredWithIris();
	}
	ActiveStreams.Remove(StreamKey);
    PendingPropertyChanges.Remove(StreamKey);

#if GORGEOUSCORE_WITH_REPLICATION_GRAPH
	if (bWasRegisteredWithGraph && FGorgeousAutoReplicationBackendTraits::UsesReplicationGraph(PreviousConfig.Backend))
	{
		if (UGorgeousAutoReplicationGraph* Graph = Cast<UGorgeousAutoReplicationGraph>(AutoReplicationGraph.Get()))
		{
			Graph->UnregisterObjectVariable(Variable);
		}
	}
#endif

#if GORGEOUSCORE_WITH_IRIS
	if (bWasRegisteredWithIris && FGorgeousAutoReplicationBackendTraits::UsesIris(PreviousConfig.Backend))
	{
		if (IrisBackend.IsValid())
		{
			IrisBackend->UnregisterStream(Variable);
		}
	}
#endif
}

void FGorgeousAutoReplicationCoordinator::NotifyRPCBroadcast(const FGorgeousQueuedRPC& QueuedRPC, UGorgeousObjectVariable* TargetVariable)
{
	if (!TargetVariable)
	{
		return;
	}

	if (FGorgeousAutoReplicationActiveStream* Stream = ActiveStreams.Find(FObjectKey(TargetVariable)))
	{
		const UEnum* BackendEnum = StaticEnum<EGorgeousAutoReplicationBackend>();
		const FString BackendLabel = BackendEnum ? BackendEnum->GetNameStringByValue(static_cast<int64>(Stream->Config.Backend)) : FString::Printf(TEXT("%d"), static_cast<int32>(Stream->Config.Backend));
		const FString ChannelLabel = Stream->RootNetworkChannel.IsNone() ? TEXT("Default") : Stream->RootNetworkChannel.ToString();
		GT_I_LOG("GT.AutoReplication.Coordinator.RPC.Routed", TEXT("AutoReplication – Routed RPC %s for %s using backend %s (channel: %s)."),
			*QueuedRPC.Payload.HandlerName.ToString(), *TargetVariable->GetName(), *BackendLabel, *ChannelLabel);
	}
}

void FGorgeousAutoReplicationCoordinator::Tick(float DeltaSeconds)
{
	UWorld* World = CachedWorld.Get();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return;
	}

	for (auto It = ActiveStreams.CreateIterator(); It; ++It)
	{
		if (!It->Value.Target.IsValid())
		{
			It.RemoveCurrent();
		}
	}

#if GORGEOUSCORE_WITH_IRIS
	if (IrisBackend.IsValid()) IrisBackend->Tick(DeltaSeconds);
#endif
    TArray<FObjectKey> Scheduled;
    PendingPropertyChanges.GenerateKeyArray(Scheduled);
    Algo::Sort(Scheduled,[this](const FObjectKey& A,const FObjectKey& B) {
        const auto* SA=ActiveStreams.Find(A); const auto* SB=ActiveStreams.Find(B);
        return (SA ? SA->Config.Priority : 0) > (SB ? SB->Config.Priority : 0);
    });
    for (const FObjectKey& Key:Scheduled)
    {
        auto* Stream=ActiveStreams.Find(Key); auto* Pending=PendingPropertyChanges.Find(Key);
        UGorgeousObjectVariable* Variable=Stream ? Stream->Target.Get() : nullptr;
        if (!Variable || !Pending) { PendingPropertyChanges.Remove(Key); continue; }
        Pending->Budget.Refill(DeltaSeconds,Stream->Config.BandwidthBudgetKB);
        Pending->TimeSinceDispatch+=FMath::Max(0.0f,DeltaSeconds);
        if (Pending->Properties.IsEmpty()) continue;
        const double Frequency=Stream->Config.GetEffectiveUpdateFrequency();
        if ((Frequency>0 && Pending->TimeSinceDispatch<1.0/Frequency) || !Pending->Budget.CanDispatch(Stream->Config.BandwidthBudgetKB)) continue;
        int64 Bytes=0;
        if (Variable->TryServerReplicateProperties(Pending->Properties,nullptr,&Bytes))
        {
            Pending->Budget.Consume(Bytes); Pending->Properties.Reset(); Pending->TimeSinceDispatch=0;
        }
    }
}

bool FGorgeousAutoReplicationCoordinator::HandleTickerTick(float DeltaSeconds)
{
	Tick(DeltaSeconds);
	return true;
}

void FGorgeousAutoReplicationCoordinator::MarkStreamDirty(UGorgeousObjectVariable* Variable)
{
#if GORGEOUSCORE_WITH_IRIS
	if (!Variable)
	{
		return;
	}

	if (!bIrisInitialized || !IrisBackend.IsValid())
	{
		return;
	}

	IrisBackend->MarkStreamDirty(Variable);
#endif
}

UReplicationGraph* FGorgeousAutoReplicationCoordinator::GetOrCreateReplicationGraph(UWorld* World)
{
#if GORGEOUSCORE_WITH_REPLICATION_GRAPH
	if (!bIsInitializingReplicationGraph)
	{
		Initialize(World);
	}

	if (!World)
	{
		return nullptr;
	}

	UNetDriver* NetDriver = World->GetNetDriver();
	if (!NetDriver)
	{
		return AutoReplicationGraph.Get();
	}

	if (NetDriver->IsUsingIrisReplication() && !bWarnedReplicationGraphIrisConflict)
	{
		//@TODO: Iris and Replication Graph cannot be used together
		bWarnedReplicationGraphIrisConflict = true;
		GT_I_LOG("GT.AutoReplication.Coordinator.Graph.IrisConflict", TEXT("NetDriver %s is using Iris replication; Iris and Replication Graph cannot be used together."), *NetDriver->GetName());
	}

	if (UReplicationGraph* ExistingGraph = NetDriver->GetReplicationDriver<UReplicationGraph>())
	{
		if (!AutoReplicationGraph.IsValid() || AutoReplicationGraph.Get() != ExistingGraph)
		{
			AutoReplicationGraph = ExistingGraph;
			bReplicationGraphInitialized = true;
			RefreshReplicationGraphStreams();
		}
		return ExistingGraph;
	}

	const UGorgeousAutoReplicationSettings* Settings = UGorgeousAutoReplicationSettings::Get();
	if (!Settings || !Settings->bEnableReplicationGraph || World->GetNetMode() == NM_Client || NetDriver->GetReplicationDriver())
	{
		return nullptr;
	}

	//Respect an override class if the QoL API provided one; otherwise fall back to settings
	//and finally to the default UGorgeousAutoReplicationGraph. 
	UClass* GraphClassPtr = AutoReplicationGraphClassOverride ? AutoReplicationGraphClassOverride.Get() : Settings->AutoReplicationGraphClass.LoadSynchronous();
	if (!GraphClassPtr)
	{
		GraphClassPtr = UGorgeousAutoReplicationGraph::StaticClass();
	}

	TSubclassOf<UReplicationGraph> GraphClass = GraphClassPtr;

	if (!GraphClass)
	{
		GT_W_LOG("GT.AutoReplication.Coordinator.Graph.ClassMissing", TEXT("Unable to resolve AutoReplication graph class while initializing NetDriver %s."), *NetDriver->GetName());
		return nullptr;
	}

	UReplicationGraph* NewGraph = NewObject<UReplicationGraph>(NetDriver, GraphClass);
	AutoReplicationGraph = NewGraph;
	NetDriver->SetReplicationDriver(NewGraph);
	bReplicationGraphInitialized = AutoReplicationGraph.IsValid();
	RefreshReplicationGraphStreams();
	return NewGraph;
#else
	return nullptr;
#endif
}

void FGorgeousAutoReplicationCoordinator::InitializeReplicationGraph(UWorld* World)
{
#if GORGEOUSCORE_WITH_REPLICATION_GRAPH
	if (bReplicationGraphInitialized || !World)
	{
		return;
	}

	if (bIsInitializingReplicationGraph)
	{
		return;
	}

	bIsInitializingReplicationGraph = true;
	GetOrCreateReplicationGraph(World);
	bIsInitializingReplicationGraph = false;
	bReplicationGraphInitialized = AutoReplicationGraph.IsValid();
#endif
}

void FGorgeousAutoReplicationCoordinator::InitializeIris(UWorld* World)
{
#if GORGEOUSCORE_WITH_IRIS
	if (bIrisInitialized || !World)
	{
		return;
	}

	if (!IrisBackend.IsValid())
	{
		IrisBackend = MakeUnique<FGorgeousAutoReplicationIrisBackend>();
	}

	if (IrisBackend.IsValid())
	{
		IrisBackend->Initialize(World);
	}

	bIrisInitialized = true;
#else
	UE_LOG(LogTemp, Verbose, TEXT("AutoReplication Iris support is disabled because Iris is not part of this build."));
#endif
}

// Ensure the destructor is defined out-of-line in the .cpp where FGorgeousAutoReplicationIrisBackend
// is a complete type so that TUniquePtr can safely delete its managed object without MSVC C4150.
FGorgeousAutoReplicationCoordinator::~FGorgeousAutoReplicationCoordinator()
{
	// Reuse TearDown to clean up resources.
	TearDown();
}

#if GORGEOUSCORE_WITH_REPLICATION_GRAPH
void FGorgeousAutoReplicationCoordinator::RefreshReplicationGraphStreams()
{
	UGorgeousAutoReplicationGraph* Graph = Cast<UGorgeousAutoReplicationGraph>(AutoReplicationGraph.Get());
	if (!Graph)
	{
		return;
	}

	for (TPair<FObjectKey, FGorgeousAutoReplicationActiveStream>& Pair : ActiveStreams)
	{
		FGorgeousAutoReplicationActiveStream& Stream = Pair.Value;
		if (!Stream.Target.IsValid())
		{
			continue;
		}

		if (Stream.IsRegisteredWithReplicationGraph())
		{
			continue;
		}

		if (FGorgeousAutoReplicationBackendTraits::UsesReplicationGraph(Stream.Config.Backend))
		{
			Graph->RegisterObjectVariable(Stream.Target.Get());
			Stream.SetRegisteredWithReplicationGraph(true);
		}
	}
}
#endif

void FGorgeousAutoReplicationCoordinator::QueuePropertyChanges(UGorgeousObjectVariable* Variable,const TSet<FName>& Properties)
{
    if (!Variable || Properties.IsEmpty() || !CachedWorld.IsValid() || CachedWorld->GetNetMode()==NM_Client) return;
    const FObjectKey Key(Variable);
    if (!ActiveStreams.Contains(Key)) RegisterObjectVariable(Variable,Variable->GetAutoReplicationConfig());
    auto& Pending=PendingPropertyChanges.FindOrAdd(Key);
    Pending.Properties.Append(Properties);
}
