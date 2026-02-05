# 🔁 AutoReplication (Runtime)

???+ info "Short Description"

    AutoReplication is the Gorgeous Core networking layer for Object Variables, enabling property streaming and RPC-style payloads with client/server routing.

??? info "Long Description"

    AutoReplication connects Gorgeous Object Variables to a higher-level replication system that supports:

    * Property payload replication (streamed, condition-based).
    * RPC-style payload dispatching with reliability and targeting.
    * Server authoritative updates with client-originated requests.
    * Blueprint-friendly registration and helper libraries.

    The system uses a mixin-based design that binds to Quality-of-Life classes (Game State, Player Controller, Player State, World Settings) and handles runtime replication with coordinator + relay/transporter components.

## 🧱 Core Concepts

* **Mixin-driven replication** – `FGorgeousAutoReplicationMixin` binds to a runtime owner and manages streams.
* **Coordinator** – `FGorgeousAutoReplicationCoordinator` schedules and applies payload replication.
* **Relay + Transporter** – RPC/payload routing between client/server.
* **Object Variable integration** – Variables can opt into AutoReplication and define custom payloads.

## 📌 Typical Use Cases

* Syncing runtime data containers across the network.
* Authoritative updates from the server with client-originated requests.
* Lightweight RPC payloads for object-variable events.

## 🔒 Notes

* AutoReplication respects access policy rules on Object Variables.
* AutoReplication payloads are serialized by Object Variables, not by the relay or transporter.
