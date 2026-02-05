# 🧩 Insight Matrix Subsystem

???+ info "Short Description"

    The subsystem is the runtime registry and execution layer for Insight Matrix providers.

## 🧱 Subsystem Responsibilities

* Register/unregister providers at runtime.
* Resolve providers by name.
* Expose active providers for scenario discovery.
* Provide access to Insight Matrix configuration paths.

## 📌 Primary API

* **UGorgeousInsightMatrixSubsystem::Get()** – Access the singleton subsystem.
* **RegisterProvider / UnregisterProvider** – Manage providers.
* **FindProvider** – Lookup by name.
* **GetInsightMatrixIniPath** – Configuration path helper.

## 🧭 Provider Lifecycle

1. A module instantiates a provider.
2. The provider registers with the subsystem.
3. The subsystem exposes the provider to harness and tooling.
