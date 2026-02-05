# 🎛️ Insight Matrix Harness & Test Matrix

???+ info "Short Description"

    Harness utilities and request structures for executing Insight Matrix scenario runs.

## 🧱 Harness Utilities

* **FGorgeousInsightHarness::StartHarness** – Starts a test matrix run.
* **FGorgeousInsightHarness::WriteReportSummary** – Writes summary files.
* **FGorgeousInsightHarness::WriteReportDetails** – Writes detailed results.

## 📑 Request Structures

* **FGorgeousInsightMatrixRequest** – Base request describing filters, output, and execution settings.
* **FGorgeousInsightScenarioDescriptor** – Descriptor for a scenario.
* **FGorgeousInsightScenarioRunResult** – Results per scenario.

## ✅ Notes

* Harness output is stored under Saved/Automation/InsightMatrix.
* Use the subsystem to access registered providers before running a matrix.
