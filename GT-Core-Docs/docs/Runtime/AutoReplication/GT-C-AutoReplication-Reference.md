# 📚 AutoReplication Reference

???+ info "Short Description"

    Reference index for AutoReplication classes, types, and Blueprint libraries.

## 🧩 Core Types

* **FGorgeousAutoReplicationStreamConfig** – Stream tuning and access policy.
* **FGorgeousAutoReplicationContext** – Runtime context for active replication.
* **FGorgeousQueuedRPC** / **FGorgeousRPCPayload** – RPC payload structures.
* **FGorgeousAutoReplicationPropertyPayload** – Serialized property payload batch.

## 🧱 Core Classes

* **FGorgeousAutoReplicationMixin** – Binds a runtime owner and executes payload/RPC routing.
* **FGorgeousAutoReplicationCoordinator** – Manages replication streams and scheduling.
* **UGorgeousAutoReplicationWorldSubsystem** – Runtime subsystem that owns AutoReplication state.
* **UGorgeousAutoReplicationRPCRelayComponent** – Client→server relay for payloads and RPCs.
* **UGorgeousAutoReplicationRPCTransporter** – Server/client/multicast routing.
* **UGorgeousAutoReplicationRPCRequestAsyncAction** – Async Blueprint action for RPC requests.

## 🧰 Blueprint Libraries

* **UGorgeousAutoReplicationNetworkingLibrary** – High-level Blueprint helpers.
* **UGorgeousAutoReplicationRPCPayloadLibrary** – RPC payload construction helpers.
* **UGorgeousAutoReplicationRPCExamples** – Example payload patterns.

## 🧾 Object Variable Integration

* **UGorgeousObjectVariable** – Registers properties for AutoReplication and handles payload serialization.
* **UGorgeousRPC_OV** – Object variable used to model RPC payloads.

## 🧭 Related Files

* Source/GorgeousCoreRuntime/Public/AutoReplication/
* Source/GorgeousCoreRuntime/Public/ObjectVariables/
