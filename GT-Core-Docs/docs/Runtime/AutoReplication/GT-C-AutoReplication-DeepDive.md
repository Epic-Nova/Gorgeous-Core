# 🔬 AutoReplication Deep Dive

???+ info "Overview"

    This document provides an in-depth technical exploration of the AutoReplication system's architecture, data flow, and internal mechanisms. It's intended for developers who need to understand the system at the implementation level.

---

## 🏗️ Architectural Philosophy

The AutoReplication system is designed around three core principles:

1. **Separation of Concerns**: Serialization happens only in Object Variables, while transport is handled by dedicated components
2. **Mixin-Based Binding**: Non-invasive attachment to existing QoL classes without inheritance requirements
3. **Dual-Path Routing**: Client→Server uses Relay components; Server→Client/Multicast uses Transporter components

```mermaid
graph TB
    subgraph "Design Layers"
        L1[Application Layer<br/>Object Variables & Handlers]
        L2[Binding Layer<br/>Mixin & Entries]
        L3[Transport Layer<br/>Relay & Transporter]
        L4[Network Layer<br/>Unreal RPC System]
    end
    
    L1 --> L2
    L2 --> L3
    L3 --> L4
    
    style L1 fill:#4a5568
    style L2 fill:#2d3748
    style L3 fill:#1a202c
    style L4 fill:#0d1117
```

---

## 🔄 Data Flow Architecture

### Property Replication Pipeline

The property replication pipeline transforms in-memory property values into network-transportable byte arrays and back.

```mermaid
flowchart TB
    subgraph "Sender (Authority)"
        A1[Object Variable]
        A2[RegisteredReplicatedProperties]
        A3[BuildAutoReplicationPropertyPayload]
        A4[SerializePropertyValue]
        A5[FGorgeousAutoReplicationPropertyEnvelope]
    end
    
    subgraph "Network"
        N1[RPC Channel]
    end
    
    subgraph "Receiver (Remote)"
        B1[HandleTransportedPropertyPayload]
        B2[ApplyAutoReplicationPropertyPayload]
        B3[DeserializePropertyValue]
        B4[Fire RepNotify]
        B5[Object Variable Updated]
    end
    
    A1 -->|Dirty Check| A2
    A2 -->|For each property| A3
    A3 -->|Mode-based| A4
    A4 --> A5
    A5 -->|Relay/Transporter| N1
    N1 --> B1
    B1 -->|Resolve Entry| B2
    B2 -->|Mode-based| B3
    B3 -->|Shadow Compare| B4
    B4 --> B5
```

### Serialization Mode Decision Tree

```mermaid
flowchart TD
    START[Serialize Property] --> MODE{Replication Mode?}
    
    MODE -->|EProperty| P1[FMemoryWriter]
    P1 --> P2[FBinaryArchiveFormatter]
    P2 --> P3[FStructuredArchive]
    P3 --> P4[FProperty::SerializeItem]
    P4 --> DONE[TArray&lt;uint8&gt;]
    
    MODE -->|ENetSerialize| N1[FMemoryWriter]
    N1 --> N2[FProperty::NetSerializeItem]
    N2 --> N3[Uses PackageMap for UObject refs]
    N3 --> DONE
    
    MODE -->|ECustomPayload| C1[BuildCustomAutoReplicationPayload]
    C1 --> C2[User-defined serialization]
    C2 --> DONE
```

---

## 🎯 RPC Routing Engine

### Route Resolution Algorithm

When an RPC is requested, the system must determine the optimal routing path:

```mermaid
flowchart TD
    START[RequestRPC Called] --> ENABLED{Networking<br/>Enabled?}
    ENABLED -->|No| FAIL[Return false]
    
    ENABLED -->|Yes| RESOLVE[ResolveRPCDestination]
    RESOLVE --> FOUND{Target<br/>Found?}
    FOUND -->|No| FAIL
    
    FOUND -->|Yes| BUILD[Build FGorgeousQueuedRPC]
    BUILD --> LOCATION{Caller<br/>Location?}
    
    LOCATION -->|Client| CDIR{RPC Direction?}
    CDIR -->|Server/Client/Multicast| RELAY[Use Relay Component]
    RELAY --> RVALID{Relay<br/>Valid?}
    RVALID -->|Yes| RSEND[RelayRPCToServer]
    RVALID -->|No| TRANS
    
    LOCATION -->|Server| TRANS[Use Transporter]
    TRANS --> TVALID{Transporter<br/>Valid?}
    TVALID -->|No| INIT[InitializeTransporter]
    INIT --> TCREATE{Created?}
    TCREATE -->|No| LOCAL[Execute Locally]
    TCREATE -->|Yes| ROUTE
    
    TVALID -->|Yes| ROUTE[RouteRPC]
    ROUTE --> SUCCESS[Return true]
    LOCAL --> SUCCESS
    RSEND --> SUCCESS
```

### RPC Type Routing Matrix

| RPC Type | Client Caller | Server Caller |
|:---------|:--------------|:--------------|
| `ReliableServer` | Relay → Server | Execute Locally |
| `UnreliableServer` | Relay → Server | Execute Locally |
| `ReliableClient` | Relay → Server → Client | Transporter → Client |
| `UnreliableClient` | Relay → Server → Client | Transporter → Client |
| `ReliableMulticast` | Relay → Server → All | Transporter → All |
| `UnreliableMulticast` | Relay → Server → All | Transporter → All |

### RPC Handler Invocation

When an RPC arrives at its target, the system must resolve and invoke the appropriate handler function:

```mermaid
flowchart TD
    INVOKE[InvokeNativeAutoReplicationRPCHandler] --> FIND[FindFunctionByName]
    FIND --> EXISTS{Function Exists?}
    EXISTS -->|No| FALSE[Return false]
    
    EXISTS -->|Yes| PARAMS{Has Parameters?}
    PARAMS -->|No| SIMPLE[ProcessEvent - nullptr]
    
    PARAMS -->|Yes| ALLOC[Allocate Parameter Storage]
    ALLOC --> INIT[InitializeStruct]
    INIT --> BINDLOOP[For Each Parameter]
    
    BINDLOOP --> ARG{Find Argument by Name?}
    ARG -->|No| FAIL[Log Warning, Abort]
    ARG -->|Yes| COPY[CopyArgumentToProperty]
    COPY --> NEXT{More Params?}
    NEXT -->|Yes| BINDLOOP
    NEXT -->|No| EXEC[ProcessEvent]
    
    SIMPLE --> CLEANUP[DestroyStruct]
    EXEC --> CLEANUP
    CLEANUP --> TRUE[Return true]
```

### Argument Type Matching

```mermaid
flowchart TD
    MATCH[CopyArgumentToProperty] --> SRCPROP[Find Source Value Property]
    SRCPROP --> VALID{Valid?}
    VALID -->|No| FAIL[Return false]
    
    VALID -->|Yes| SAMETYPE{Same Type?}
    SAMETYPE -->|Yes| COPY[CopyCompleteValue]
    SAMETYPE -->|No| MISMATCH[Log Type Mismatch]
    MISMATCH --> FAIL
    
    COPY --> SUCCESS[Return true]
```

---

## 🔗 Mixin Binding Mechanism

### Binding Lifecycle

```mermaid
stateDiagram-v2
    [*] --> Unbound: Constructor
    Unbound --> Bound: Bind()
    Bound --> Initialized: InitializeAdditionalData()
    Initialized --> Active: RefreshCachedValues()
    Active --> Active: OnRep triggers refresh
    Active --> Unbound: Owner destroyed
    Unbound --> [*]
```

### Entry Resolution Flow

The mixin maintains two parallel data structures that must stay synchronized:

```mermaid
flowchart LR
    subgraph "Design-Time Data"
        AD[AdditionalData<br/>TMap&lt;FName, FGorgeousObjectVariableEntry&gt;]
    end
    
    subgraph "Runtime Replicated Data"
        RV[ReplicatedVariables<br/>TArray&lt;FGorgeousReplicatedVariableEntry&gt;]
    end
    
    subgraph "Index Mapping"
        KR[KeyToReplicationIndex<br/>TMap&lt;FName, uint16&gt;]
    end
    
    AD <-->|RefreshCachedValues| RV
    AD -->|GetOrAssignReplicationIndex| KR
    KR -->|Lookup| RV
```

### Handle Caching Strategy

```mermaid
flowchart TD
    CACHE[CacheValue Called] --> CHECK{Variable<br/>Valid?}
    CHECK -->|No| RESET[Reset Handle]
    CHECK -->|Yes| BIND[SetAutoReplicationBinding]
    
    BIND --> CONFIG{Stream<br/>Override?}
    CONFIG -->|Yes| APPLY[Apply StreamConfigOverride]
    CONFIG -->|No| DEFAULT[Use Default Config]
    
    APPLY --> ENABLE{Auto Enable<br/>Networking?}
    DEFAULT --> ENABLE
    
    ENABLE -->|Yes| ACTIVATE[ActivateReplication]
    ENABLE -->|No| STORE[Store Reference Only]
    
    ACTIVATE --> REGISTER[Register with Coordinator]
    REGISTER --> DONE[Handle Ready]
    STORE --> DONE
```

---

## 🌐 Network Topology

### Component Distribution

```mermaid
flowchart TB
    subgraph "Server"
        SG[GameState]
        SGM[Mixin]
        ST[Transporter]
        SC[Coordinator]
    end
    
    subgraph "Client 1"
        C1G[GameState Proxy]
        C1M[Mixin]
        C1T[Transporter Proxy]
        C1R[Relay on PC]
    end
    
    subgraph "Client 2"
        C2G[GameState Proxy]
        C2M[Mixin]
        C2T[Transporter Proxy]
        C2R[Relay on PC]
    end
    
    SG --> SGM
    SGM --> ST
    SGM --> SC
    
    C1G --> C1M
    C1M --> C1T
    C1M --> C1R
    
    C2G --> C2M
    C2M --> C2T
    C2M --> C2R
    
    ST -.->|Replicates| C1T
    ST -.->|Replicates| C2T
    
    C1R -->|Server RPCs| SG
    C2R -->|Server RPCs| SG
    
    ST -->|Client RPCs| C1R
    ST -->|Client RPCs| C2R
```

### RPC Execution Sequence

```mermaid
sequenceDiagram
    participant C as Client Code
    participant CM as Client Mixin
    participant CR as Client Relay
    participant NET as Network
    participant ST as Server Transporter
    participant SM as Server Mixin
    participant SOV as Server Object Variable
    
    C->>CM: RequestRPC(Server)
    CM->>CM: Build QueuedRPC
    CM->>CR: RelayRPCToServer()
    CR->>NET: ServerRelayRPCReliable
    NET->>ST: Receive
    ST->>SM: HandleTransportedRPC
    SM->>SM: ExecuteAutoReplicationRPC
    SM->>SOV: InvokeNativeAutoReplicationRPCHandler
    SOV->>SOV: ProcessEvent(Handler)
    SOV-->>SM: Result
    SM-->>ST: EmitResult
    Note over ST: NotifyRequestCompleted
```

---

## 🔒 Thread Safety Considerations

### Concurrent Access Points

```mermaid
flowchart TB
    subgraph "Game Thread"
        GT1[Property Registration]
        GT2[RPC Dispatch]
        GT3[Payload Building]
    end
    
    subgraph "Network Thread"
        NT1[RPC Reception]
        NT2[Payload Delivery]
    end
    
    subgraph "Shared State"
        SS1[RegisteredReplicatedProperties]
        SS2[PendingRPCs]
        SS3[CachedValues]
    end
    
    GT1 --> SS1
    GT2 --> SS2
    GT3 --> SS3
    
    NT1 -.-> SS2
    NT2 -.-> SS3
    
    style SS1 fill:#744210
    style SS2 fill:#744210
    style SS3 fill:#744210
```

!!! warning "Threading Model"
    
    AutoReplication assumes single-threaded access from the Game Thread. All network callbacks are marshaled to the Game Thread before processing. Do not access mixin state from async tasks.

---

## 📊 Performance Characteristics

### Payload Size Estimation

```
PropertyPayload Size = Σ(PropertySize + MetadataOverhead)
  where MetadataOverhead ≈ 32 bytes per property
    - FName: 8 bytes
    - Mode: 1 byte
    - Condition: 1 byte
    - Flags: 2 bytes
    - Length prefix: 4 bytes
    - Padding: ~16 bytes
```

### Replication Index Capacity

- Maximum entries per mixin: **65,534** (uint16 - 1 for invalid)
- Index assignment: Sequential, never reused during session
- Overflow behavior: Ensure fires, returns InvalidReplicationIndex

### Memory Footprint

| Component | Approximate Size |
|:----------|:-----------------|
| Mixin (base) | ~128 bytes |
| Per entry overhead | ~64 bytes |
| Transporter component | ~256 bytes |
| Relay component | ~128 bytes |
| Per queued RPC | ~96 bytes + payload |

---

## 🧪 Debugging Internals

### Diagnostic Checklist

```mermaid
flowchart TD
    ISSUE[Issue Detected] --> CAT{What type of issue?}
    
    CAT --> BINDING[Binding Issues]
    CAT --> REPLICATION[Replication Issues]
    CAT --> RPC[RPC Issues]
    
    BINDING --> B1[1. Check Bind called]
    B1 --> B2[2. Check bIsBound flag]
    B2 --> B3[3. Verify storage pointers]
    
    REPLICATION --> R1[1. Check bNetworkingEnabled]
    R1 --> R2[2. Verify entry bReplicate]
    R2 --> R3[3. Check coordinator registration]
    
    RPC --> RPC1[1. Verify handler exists]
    RPC1 --> RPC2[2. Check relay/transporter]
    RPC2 --> RPC3[3. Trace network path]
```

---

## ⚙️ Configuration Points

### Stream Configuration Hierarchy

```mermaid
flowchart TD
    DEFAULT[UGorgeousAutoReplicationSettings::DefaultStreamConfig]
    ENTRY[FGorgeousObjectVariableEntry::StreamConfigOverride]
    VARIABLE[UGorgeousObjectVariable::AutoReplicationConfig]
    
    DEFAULT -->|Base| ENTRY
    ENTRY -->|bOverrideStreamConfig| VARIABLE
    VARIABLE -->|Final| ACTIVE[Active Configuration]
    
    style ACTIVE fill:#22543d
```

### Runtime Override Points

| Setting | Location | Scope |
|:--------|:---------|:------|
| Default stream config | Project Settings | Global |
| Entry stream override | QoL class definition | Per-entry |
| Variable config | Object Variable instance | Per-variable |
| Access policy | `SetNetworkAccessPolicy()` | Per-variable |
| Respect access | `SetAutoReplicationRespectAccessPolicy()` | Per-variable |

---

## 🔮 Extension Points

### Custom Serialization

Override `BuildCustomAutoReplicationPayload` and `ApplyCustomAutoReplicationPayload` in your Object Variable class to implement custom serialization logic.

### Custom RPC Handlers

Implement functions matching the handler name in your Object Variable class. The system uses reflection to invoke handlers with automatic argument binding.

### Custom Transporter Routing

Subclass `UGorgeousAutoReplicationRPCTransporter` and override the virtual routing functions to implement custom routing logic.

### Coordinator Integration

Access `FGorgeousAutoReplicationCoordinator::Get(World)` for Iris/RepGraph integration points.

---

## ⚠️ Known Limitations

!!! danger "Critical Constraints"
    
    1. **No cross-world replication**: Variables must exist in the same UWorld context
    2. **No late binding**: Entries must be configured before `InitializeAdditionalData()`
    3. **No transporter without authority**: Clients cannot create transporters
    4. **PackageMap required for UObject refs**: ENetSerialize mode needs valid connection

!!! info "Design Tradeoffs"
    
    - Favors simplicity over micro-optimizations
    - Prioritizes debuggability over minimal bandwidth
    - Assumes reliable ordering for property payloads
