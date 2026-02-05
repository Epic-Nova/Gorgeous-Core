# 🔬 Object Variables Deep Dive

???+ info "Overview"

    This document provides an in-depth technical exploration of the Object Variable and Root Object Variable system's architecture, lifecycle management, and internal mechanisms. It focuses on the core variable system itself - for networking details, see the [AutoReplication Deep Dive](../AutoReplication/GT-C-AutoReplication-DeepDive.md).

---

## 🏗️ Architectural Foundation

The Object Variable system is built on a hierarchical registry pattern with support for persistence and cross-reference resolution.

```mermaid
graph TB
    subgraph "Core Architecture"
        ROOT[Root Object Variable]
        REG[Variable Registry]
        ID[Identifier System]
        SERIAL[Serialization]
    end
    
    subgraph "Support Systems"
        PERSIST[Persistence]
        SNAPSHOT[Snapshot System]
        HIERARCHY[Parent-Child Relations]
    end
    
    ROOT --> REG
    REG --> ID
    ID --> SERIAL
    
    SERIAL --> PERSIST
    SERIAL --> SNAPSHOT
    ROOT --> HIERARCHY
    
    style ROOT fill:#4a5568
    style REG fill:#2d3748
```

---

## 🌳 Hierarchy Model

### Root Object Variable Pattern

All Object Variables exist within a hierarchy rooted at `UGorgeousRootObjectVariable` instances. The Root acts as a namespace and registry manager.

```mermaid
flowchart TD
    subgraph "Global Registry"
        GR[Static Variable Map]
    end
    
    subgraph "Root: Default"
        R1[UGorgeousRootObjectVariable]
        R1C1[Game Variables]
        R1C2[Player Variables]
        R1C3[Level Variables]
    end
    
    subgraph "Root: Custom"
        R2[UGorgeousRootObjectVariable]
        R2C1[Mod Variables]
        R2C2[Plugin Variables]
    end
    
    GR --> R1
    GR --> R2
    
    R1 --> R1C1
    R1 --> R1C2
    R1 --> R1C3
    
    R2 --> R2C1
    R2 --> R2C2
    
    R1C1 --> V1[Integer_OV]
    R1C1 --> V2[String_OV]
    R1C2 --> V3[Transform_OV]
```

### Parent-Child Relationships

```mermaid
flowchart TD
    subgraph "Ownership Chain"
        ROOT[Root Object Variable]
        PARENT[Parent Variable]
        CHILD[Child Variable]
        GRANDCHILD[Grandchild Variable]
    end
    
    ROOT -->|Contains| PARENT
    PARENT -->|Contains| CHILD
    CHILD -->|Contains| GRANDCHILD
    
    GRANDCHILD -.->|GetRoot| ROOT
    CHILD -.->|GetParent| PARENT
```

### Registry Resolution Algorithm

```mermaid
flowchart TD
    START[RegisterWithRegistry] --> UNIQUE{bUnique?}
    
    UNIQUE -->|Yes| POLICY{UniqueRegistrationPolicy?}
    POLICY -->|RemoveAllExisting| REMOVE[Remove all same-class instances]
    REMOVE --> REGISTER
    POLICY -->|CancelRegistration| CHECK[Check for collisions]
    CHECK --> FOUND{Collision Found?}
    FOUND -->|Yes| CANCEL[Log warning, return false]
    FOUND -->|No| REGISTER
    
    UNIQUE -->|No| REGISTER[Add to VariableRegistry]
    REGISTER --> TRACK[TrackRegisteredVariable]
    TRACK --> DONE[Registration Complete]
```

---

## 🆔 Identifier System

### GUID Generation & Resolution

Every Object Variable has a unique identifier (`FGuid`) for cross-reference and lookup:

```mermaid
flowchart TD
    subgraph "Creation Path"
        CREATE[NewObjectVariable] --> GENGUID[FGuid::NewGuid]
        GENGUID --> ASSIGN[UniqueIdentifier = GUID]
        ASSIGN --> REGISTER[Register in Root]
    end
    
    subgraph "Lookup Path"
        LOOKUP[FindVariableByIdentifier] --> SEARCH[Search All Roots]
        SEARCH --> FOUND{Found?}
        FOUND -->|Yes| RETURN[Return Variable]
        FOUND -->|No| NULL[Return nullptr]
    end
```

### Display Name Management

Display names provide human-readable identification with automatic uniqueness handling:

```mermaid
sequenceDiagram
    participant V as Variable
    participant R as Root
    participant N as Name Registry
    
    V->>R: SetDisplayName("MyVar")
    R->>N: ReleaseDisplayName(Variable)
    R->>N: ReserveDisplayName(Variable, "MyVar")
    
    alt Name Available
        N-->>R: "MyVar"
    else Name Taken
        N-->>R: "MyVar-2"
    end
    
    R-->>V: Assigned Name
    V->>V: NotifyDisplayNameChanged()
```

### Name Uniqueness Algorithm

```mermaid
flowchart TD
    REQUEST[ReserveDisplayName] --> CHECK{Name Exists?}
    CHECK -->|No| ASSIGN[Assign Name]
    CHECK -->|Yes| SUFFIX[Try Name-2]
    SUFFIX --> CHECK2{Name-2 Exists?}
    CHECK2 -->|No| ASSIGN2[Assign Name-2]
    CHECK2 -->|Yes| INCREMENT[Increment Suffix]
    INCREMENT --> SUFFIX
    ASSIGN --> DONE[Return Final Name]
    ASSIGN2 --> DONE
```

---

## 🔄 Lifecycle Management

### Creation Flow

```mermaid
stateDiagram-v2
    [*] --> Allocated: NewObject
    Allocated --> Rooted: AddToRoot()
    Rooted --> Identified: Assign GUID
    Identified --> Parented: Set Parent
    Parented --> Registered: RegisterWithRegistry
    Registered --> Named: SetDisplayName
    Named --> Ready: Variable Active
    Ready --> [*]: BeginDestroy
```

### Detailed Creation Sequence

```mermaid
sequenceDiagram
    participant F as Factory/Caller
    participant V as New Variable
    participant R as Root Variable
    participant REG as Registry
    
    F->>V: NewObject<UGorgeousObjectVariable>()
    V->>V: Constructor
    V->>V: AddToRoot() (prevent GC)
    V->>V: UniqueIdentifier = FGuid::NewGuid()
    F->>V: SetParent(ParentVariable)
    V->>R: GetRoot()
    V->>REG: RegisterWithRegistry()
    REG->>REG: Add to VariableRegistry array
    REG->>REG: TrackRegisteredVariable()
    F->>V: SetDisplayName("MyVar")
    V->>R: ReserveDisplayName()
    R-->>V: Unique name assigned
    Note over V: Variable now ready for use
```

### Destruction Sequence

```mermaid
flowchart TD
    DESTROY[BeginDestroy Called] --> STATS[Update Destruction Stats]
    STATS --> REGISTRY[EnsureRemovedFromRegistry]
    REGISTRY --> UNREGPARENT[Remove from Parent's Registry]
    UNREGPARENT --> DISPLAY[Release Display Name]
    DISPLAY --> CHILDREN[Notify Children of Orphaning]
    CHILDREN --> SUPER[Super::BeginDestroy]
    SUPER --> GC[Garbage Collection]
```

### Stats Tracking

The system maintains runtime statistics for debugging:

| Stat | Description |
|:-----|:------------|
| `STAT_GOV_Created` | Total variables created this session |
| `STAT_GOV_Destroyed` | Total variables destroyed this session |
| `STAT_GOV_Alive` | Currently alive count (Created - Destroyed) |

---

## 📦 Serialization System

### Snapshot Capture

Snapshots capture the complete state of a variable for persistence or duplication:

```mermaid
flowchart TD
    CAPTURE[CaptureSnapshot] --> VERSION[Write Version Header]
    VERSION --> ITERATE[For Each Property]
    
    ITERATE --> FILTER{ShouldCaptureProperty?}
    FILTER -->|No| SKIP[Skip Property]
    FILTER -->|Yes| SERIALIZE[Serialize Value]
    
    SERIALIZE --> NAME[Write Property Name]
    NAME --> TYPE[Write Type Info]
    TYPE --> DATA[Write Property Data]
    DATA --> SKIP
    
    SKIP --> MORE{More Properties?}
    MORE -->|Yes| ITERATE
    MORE -->|No| DONE[Snapshot Complete]
```

### Property Filter Criteria

```mermaid
flowchart TD
    CHECK[ShouldCaptureProperty] --> FLAGS{Has Edit or BlueprintVisible?}
    FLAGS -->|No| REJECT[Exclude]
    
    FLAGS -->|Yes| BLOCKED{Has Blocked Flags?}
    BLOCKED -->|Yes| REJECT
    
    BLOCKED -->|No| OWNER{Owner = Base Class?}
    OWNER -->|Yes| REJECT
    
    OWNER -->|No| ACCEPT[Include in Snapshot]
```

**Blocked Property Flags:**

| Flag | Reason for Exclusion |
|:-----|:---------------------|
| `CPF_Transient` | Temporary runtime data |
| `CPF_DuplicateTransient` | Not meant to be copied |
| `CPF_TextExportTransient` | Export-only data |
| `CPF_NonPIEDuplicateTransient` | PIE-specific exclusion |
| `CPF_DisableEditOnInstance` | Instance-locked |
| `CPF_EditConst` | Read-only property |

### Snapshot Restoration

```mermaid
flowchart TD
    RESTORE[RestoreSnapshot] --> HEADER[Read Version Header]
    HEADER --> COMPAT{Version Compatible?}
    COMPAT -->|No| FAIL[Return false]
    
    COMPAT -->|Yes| LOOP[For Each Saved Property]
    LOOP --> FINDPROP{Property Exists?}
    FINDPROP -->|No| SKIPLOAD[Skip, Continue]
    FINDPROP -->|Yes| DESERIALIZE[Deserialize Value]
    DESERIALIZE --> APPLY[Apply to Property]
    APPLY --> SKIPLOAD
    
    SKIPLOAD --> MOREPROP{More Properties?}
    MOREPROP -->|Yes| LOOP
    MOREPROP -->|No| SUCCESS[Return true]
```

---

## 📊 Memory Layout

### Object Variable Instance Structure

```
UGorgeousObjectVariable Memory Layout (Approximate)
├── UObject Header (~56 bytes)
│   ├── VTable pointer
│   ├── Object flags
│   ├── Index & serial number
│   └── Class pointer
├── Configuration
│   ├── bPersistent (1 byte)
│   ├── bUnique (1 byte)
│   ├── UniqueRegistrationPolicy (1 byte)
│   └── Padding (5 bytes)
├── Identifiers
│   ├── UniqueIdentifier (16 bytes - FGuid)
│   │   ├── A (4 bytes)
│   │   ├── B (4 bytes)
│   │   ├── C (4 bytes)
│   │   └── D (4 bytes)
│   └── DisplayName (~24 bytes - FString header)
├── Hierarchy
│   ├── Parent (8 bytes - TObjectPtr)
│   └── VariableRegistry (~24 bytes - TArray header)
│       ├── Data pointer (8 bytes)
│       ├── ArrayNum (4 bytes)
│       └── ArrayMax (4 bytes)
└── Subclass-specific data (varies by type)
```

### Root Object Variable Additional Data

```
UGorgeousRootObjectVariable Additional Memory
├── Base UGorgeousObjectVariable
├── RootName (8 bytes - FName)
├── DisplayNameRegistry (~48 bytes - TMap header)
├── IdentifierLookup (~48 bytes - TMap header)
└── Static Registration (~8 bytes)
```

---

## 🔍 Lookup Operations

### Find Variable by Identifier

```mermaid
flowchart TD
    START[FindVariableByIdentifier] --> ROOTS[Get All Root Variables]
    ROOTS --> ITERATE[For Each Root]
    
    ITERATE --> SEARCH[Search IdentifierLookup Map]
    SEARCH --> FOUND{Found?}
    FOUND -->|Yes| RETURN[Return Variable]
    FOUND -->|No| NEXTROOT{More Roots?}
    NEXTROOT -->|Yes| ITERATE
    NEXTROOT -->|No| NULLPTR[Return nullptr]
```

### Find Variable by Display Name

```mermaid
flowchart TD
    START[FindVariableByDisplayName] --> ROOT[Get Target Root]
    ROOT --> VALID{Root Valid?}
    VALID -->|No| NULLPTR[Return nullptr]
    
    VALID -->|Yes| LOOKUP[Search DisplayNameRegistry]
    LOOKUP --> FOUND{Found?}
    FOUND -->|Yes| RETURN[Return Variable]
    FOUND -->|No| NULLPTR
```

---

## 🧪 Debugging Tools

### Diagnostic Queries

```cpp
// Check if variable is properly registered
bool bRegistered = UGorgeousRootObjectVariable::IsVariableRegistered(Variable);

// Find variable by identifier
UGorgeousObjectVariable* Found = 
    UGorgeousRootObjectVariable::FindVariableByIdentifier(GUID);

// Get hierarchy registry for a root
TArray<UGorgeousObjectVariable*> Registry = 
    UGorgeousRootObjectVariable::GetVariableHierarchyRegistry(RootName);

// Get all variables of a specific type
TArray<UGorgeousObjectVariable*> TypedVars = 
    Root->GetAllVariablesOfClass(UMyCustomVariable::StaticClass());
```

### Registration Validation

```mermaid
flowchart TD
    CHECK[Validate Registration] --> HASROOT{Has Root?}
    HASROOT -->|No| FAIL1[ERROR: No root assigned]
    
    HASROOT -->|Yes| INREG{In Registry?}
    INREG -->|No| FAIL2[ERROR: Not in registry]
    
    INREG -->|Yes| HASGUID{Has Valid GUID?}
    HASGUID -->|No| FAIL3[ERROR: Invalid identifier]
    
    HASGUID -->|Yes| LOOKUP{GUID Lookup Works?}
    LOOKUP -->|No| FAIL4[ERROR: Lookup mismatch]
    
    LOOKUP -->|Yes| SUCCESS[Registration Valid]
```

---

## ⚠️ Known Limitations

!!! danger "Critical Constraints"
    
    1. **No circular parent references**: Parent chain must be acyclic
    2. **Single root per variable**: Variables cannot belong to multiple roots simultaneously
    3. **GUID stability**: Changing GUID requires manual re-registration
    4. **Display name uniqueness**: Names are unique per-root only

!!! info "Design Tradeoffs"
    
    - **Hierarchical registry** over flat lookup for logical organization
    - **String-based display names** for human readability vs raw GUIDs
    - **GUID-based identifiers** for stable cross-session references
    - **Lazy lookup maps** built on first access for memory efficiency
