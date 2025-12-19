// Minimal harness API for AutoReplication matrix
#pragma once

#include "GorgeousAutomationTestMatrix.h"

/** Public harness utilities for AutoReplication matrix runs. */
class GORGEOUSCORERUNTIMEUTILITIES_API FGorgeousAutoReplicationTestMatrixCore
{
public:
	/** Start shared harness resources for a matrix run. Returns true if started successfully (stub).
	 *  Implement the real spin-up (PIE/dedicated server orchestration) inside the runtime harness.
	 */
	static bool StartHarness(const FGorgeousAutomationMatrixRequest& Request);

	/** Stop shared harness resources (stub). */
	static void StopHarness();

	/** Save scenario result to disk under the given folder. Returns true on success (stubbed).
	 *  Path is relative to the project's Saved/Automation/AutoReplication directory when implemented.
	 */
	static bool SaveScenarioResult(const FGorgeousAutomationScenarioDescriptor& Descriptor, const FGorgeousAutomationScenarioResult& Result, const FString& Folder = TEXT(""));
};

#define REGISTER_GORGEOUS_AUTOREPLICATION_SCENARIO(DescriptorInit) REGISTER_GORGEOUS_AUTOMATION_SCENARIO(DescriptorInit)
