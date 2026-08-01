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
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "EditorSubsystem.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousExtensionResourceGuardEnforcer.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Extension Resource Guard Enforcer
| Functional Name: UGorgeousExtensionResourceGuardEnforcer
| Parent Class: UEditorSubsystem
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Editor subsystem that enforces Extension Resource Guard compliance.
|
| This subsystem is activated ONLY when the user actively rejects the
| prompt to restart th engine. Once activated it:
|   1. Displays a toast reminder every 2 minutes via the Gorgeous Things
|      logging system.
|   3. After 10 minutes of continued rejection, saves all unsaved assets
|      and shuts down the editor gracefully - no data is lost.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS()
class GORGEOUSCOREEDITORUTILITIES_API UGorgeousExtensionResourceGuardEnforcer : public UEditorSubsystem
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
public:

	// Initialize override to set up ticker and callbacks.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Deinitialize override to clean up ticker and callbacks.
	virtual void Deinitialize() override;
	//<------------------------------------------------------------------------->

	//<============================--- C++ Only ---=============================>

	/**
	 * Called when the user actively rejects the plugin-enable prompt.
	 * Starts the enforcement timer cycle for the given plugin.
	 *
	 * @param PluginName The plugin the user refused to enable.
	 * @param SystemDisplayName Human-readable name of the system that needs it.
	 */
	void BeginEnforcement(const FString& PluginName, const FString& SystemDisplayName);

	/**
	 * Called when the user enables the required plugin or when the issue is otherwise resolved.
	 * Stops the enforcement cycle for the given plugin.
	 *
	 * @param PluginName The plugin that is now enabled/resolved.
	 */
	void StopEnforcement(const FString& PluginName);

	/**
	 * Checks if there are any active enforcements in progress.
	 *
	 * @return Returns true if there are any active enforcements in progress, false otherwise.
	 */
	bool IsEnforcementActive() const;

private:

	/**
	 * Ticker callback that runs periodically to check enforcement status.
	 * Displays reminder toasts and triggers shutdown when deadlines are reached.
	 *
	 * @param DeltaTime Time since the last tick (not used for timing, just a standard ticker signature).
	 * @return Returns true to continue ticking, false to stop.
	 */
	bool OnTick(float DeltaTime);

	/**
	 * Runs once at subsystem initialization.
	 * Discovers ALL Extension Resource Guard data assets (native and Blueprint)
	 * via the Asset Registry and feeds them into the standard
	 * UEditorValidatorSubsystem pipeline, which triggers our
	 * UGorgeousExtensionResourceGuardValidator through the normal
	 * data validation flow.
	 */
	void PerformStartupValidation();

	// Saves all dirty packages and shuts down the editor safely.
	void SafeShutdownEditor();
	//<------------------------------------------------------------------------->

	//<============================--- Variables ---============================>

	// Represents a single enforcement session for one rejected plugin.
	struct FEnforcementEntry
	{
		FString PluginName;
		FString SystemDisplayName;

		// When the user rejected the prompt (seconds since GStartTime).
		double StartTimestamp = 0.0;

		// When we last showed a toast (seconds since GStartTime).
		double LastToastTimestamp = 0.0;

		// Whether the final shutdown warning has been shown.
		bool bShutdownWarningShown = false;
	};

	// Active enforcement entries keyed by plugin name.
	TMap<FString, FEnforcementEntry> ActiveEnforcements;

	// Ticker delegate handle for the periodic check. */
	FTSTicker::FDelegateHandle TickerHandle;


	// Toast reminder interval in seconds (2 minutes).
	static constexpr double ToastIntervalSeconds = 120.0;

	// Hard shutdown deadline in seconds after enforcement start (10 minutes).
	static constexpr double ShutdownDeadlineSeconds = 600.0;
	//<------------------------------------------------------------------------->
};