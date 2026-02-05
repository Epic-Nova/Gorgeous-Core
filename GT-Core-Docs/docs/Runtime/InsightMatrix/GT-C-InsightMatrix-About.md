# 🧠 Insight Matrix (Runtime)

???+ info "Short Description"

    Insight Matrix is a runtime test matrix and telemetry layer for orchestrating and reporting automated scenarios.

??? info "Long Description"

    Insight Matrix provides a runtime registry for scenario execution, a harness for running matrix requests, and optional UI/debug tooling. It is designed to integrate with automation workflows (including Gauntlet) and supports modular providers that publish scenario results.

## 🧱 Key Components

* **UGorgeousInsightMatrixSubsystem** – Runtime registry for providers and matrix execution.
* **IGorgeousInsightMatrixProvider** – Provider interface for publishing matrix data.
* **FGorgeousCoreInsightMatrixProvider** – Core runtime provider.
* **FGorgeousInsightHarness** – Public harness utilities for running matrices.
* **FGorgeousInsightMatrixRequest** – Request definition for scenario runs.

## 🧩 Optional UI

* **GorgeousInsightDebugPanel** – Unified debug panel scaffold.
* **Mini charts** – Bar, line, histogram, heatmap, scatter, table, etc.

## ✅ Notes

* Results are stored under Saved/Automation/InsightMatrix.
* Providers are registered via the subsystem to expose scenarios.
