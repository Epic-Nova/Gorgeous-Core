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
#include "CoreMinimal.h"
#include "DataRegistryTypes.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousGeneralSystemAction.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UGeneralSystemConfiguration_PDA;
class UBlueprint;
class IAssetEditorInstance;
class UGorgeousGeneralSystemAction;
class UGorgeousSystemCondition;
struct FGorgeousSystemActionEvaluation;
//<------------------------------------------------------------->

/**
 * Selects the execution context for a general system action.
 *
 * @author Nils Bergemann
 */
UENUM(BlueprintType)
enum class EGorgeousSystemActionTarget : uint8
{
	Game	UMETA(DisplayName = "Game", ToolTip = "Evaluate and execute this action in the game/runtime context."),
	Editor	UMETA(DisplayName = "Editor", ToolTip = "Evaluate and execute this action in the Unreal Editor context."),
	Both	UMETA(DisplayName = "Both", ToolTip = "Evaluate and execute this action in both game/runtime and editor contexts.")
};

/**
 * Selects whether a configured class must match exactly or derive from it.
 *
 * @author Nils Bergemann
 */
UENUM(BlueprintType)
enum class EGorgeousClassComparison : uint8
{
	ExactClass UMETA(DisplayName = "Exact Class", ToolTip = "The configured class must exactly match the expected class."),
	ClassOrSubclass UMETA(DisplayName = "Class Or Subclass", ToolTip = "The configured class must be the expected class or derive from it.")
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: General System Action
| Functional Name: UGorgeousGeneralSystemAction
| Parent Class: UObject
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Defines an activation step owned by a General System Configuration.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/Overview/#general-system-configuration",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousGeneralSystemAction : public UObject
{
    GENERATED_BODY()

    //<============================--- Overrides ---============================>
    #pragma region Overrides
public:

    // Implements the ShouldActivateForSystem_Implementation override.
    /** Runtime registry operations remain available; prompt/content/config mutations require an interactive editor. */
    bool CanExecuteInCurrentSession() const;

    virtual bool ShouldActivateForSystem_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration) const;

    // Implements the PredictAction_Implementation override.
    virtual bool PredictAction_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration) const;

    // Implements the ExecuteAction_Implementation override.
    virtual bool ExecuteAction_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration);

    // Implements the OnSystemActivated_Implementation override.
    virtual void OnSystemActivated_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration) {}

    // Implements the OnSystemDeactivated_Implementation override.
    virtual void OnSystemDeactivated_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration) {}
    //<------------------------------------------------------------------------->
    #pragma endregion Overrides


    //<=======================--- Blueprint Functions ---=======================>
    #pragma region Blueprint Functions
	
    /**
     * Supplies the Details panel with the names returned by the system registry API.
     *
     * @return The available general system names.
     */
    UFUNCTION()
    static TArray<FName> GetAvailableSystemNames();

    /**
     * Returns whether this action should run for the supplied system.
     *
     * @param SystemConfiguration The system that owns the action.
     * @return True when the action should run for the system.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Gorgeous Core|General Systems")
    bool ShouldActivateForSystem(UGeneralSystemConfiguration_PDA* SystemConfiguration) const;

    /**
     * Predicts whether this action can execute without changing project state.
     *
     * @param SystemConfiguration The system that owns the action.
     * @return True when the action can execute.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Gorgeous Core|General Systems")
    bool PredictAction(UGeneralSystemConfiguration_PDA* SystemConfiguration) const;

    /**
     * Activates this action after the owning system and its requirements activate.
     *
     * @param SystemConfiguration The system that owns the action.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous Core|General Systems")
    void OnSystemActivated(UGeneralSystemConfiguration_PDA* SystemConfiguration);

    /**
     * Deactivates this action when the owning system is explicitly deactivated.
     *
     * @param SystemConfiguration The system that owns the action.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous Core|General Systems")
    void OnSystemDeactivated(UGeneralSystemConfiguration_PDA* SystemConfiguration);

    /**
     * Executes this action in response to a user-selected prompt option.
     *
     * @param SystemConfiguration The system that owns the action.
     * @return True when execution succeeds.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous Core|General Systems")
    bool ExecuteAction(UGeneralSystemConfiguration_PDA* SystemConfiguration);
    //<------------------------------------------------------------------------->
    #pragma endregion Blueprint Functions


    //<====================--- UAT/UBT Exposed Variables ---====================>
    #pragma region UAT/UBT Exposed Variables
	
	/**
	 * Optional system-presence condition. This action only runs when every
	 * referenced system configuration exists. Unlike the owning PDA's required
	 * systems, missing entries here skip only this action.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Activation Condition", meta = (DisplayName = "Required Systems", GetOptions = "GetAvailableSystemNames"))
	TArray<FName> RequiredSystems;

	// Determines whether this action is evaluated in the game, editor, or both contexts.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
	EGorgeousSystemActionTarget Target = EGorgeousSystemActionTarget::Both;
    //<------------------------------------------------------------------------->
    #pragma endregion UAT/UBT Exposed Variables
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Add Data Registry Source Asset
| Functional Name: UGorgeousAddDataRegistrySourceAction
| Parent Class: UGorgeousGeneralSystemAction
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Registers an asset with a Data Registry dynamic or meta source.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName = "Add Data Registry Source Asset",
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/Overview/#general-system-configuration",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousAddDataRegistrySourceAction : public UGorgeousGeneralSystemAction
{
    GENERATED_BODY()

    //<============================--- Overrides ---============================>
    #pragma region Overrides
public:

    // Implements the OnSystemActivated_Implementation override.
    virtual void OnSystemActivated_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration) override;

    // Implements the OnSystemDeactivated_Implementation override.
    virtual void OnSystemDeactivated_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration) override;

    // Implements the PredictAction_Implementation override.
    virtual bool PredictAction_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration) const override;
    //<------------------------------------------------------------------------->
    #pragma endregion Overrides


    //<====================--- UAT/UBT Exposed Variables ---====================>
    #pragma region UAT/UBT Exposed Variables
public:

    // Registry type that owns the dynamic source to be extended.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data Registry")
    FDataRegistryType RegistryType;

    // Asset supplied to the registry source, commonly a Data Table or Curve Table.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data Registry", meta = (AllowedClasses = "/Script/Engine.DataTable,/Script/Engine.CurveTable"))
    TSoftObjectPtr<UObject> SourceAsset;

    // Higher priorities are searched before lower priorities by Data Registry meta sources.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data Registry")
    int32 Priority = 0;

    //<------------------------------------------------------------------------->
    #pragma endregion UAT/UBT Exposed Variables
};


/**
 * Selects the operation applied to a configuration value.
 *
 * @author Nils Bergemann
 */
UENUM(BlueprintType)
enum class EGorgeousConfigOperation : uint8
{
	Read	UMETA(DisplayName = "Read", ToolTip = "Read the current value without changing the configuration."),
	NewLine	UMETA(DisplayName = "New Line", ToolTip = "Write a normal configuration line using the key and value."),
	SetOrNewLine UMETA(DisplayName = "Set Or New Line", ToolTip = "Set the existing value, or write a new line when the key is not present."),
	Add		UMETA(DisplayName = "Add", ToolTip = "Add the configuration value with Unreal's plus prefix."),
	Remove	UMETA(DisplayName = "Remove", ToolTip = "Remove the configuration value with Unreal's minus prefix."),
	RemoveLine UMETA(DisplayName = "Remove Line", ToolTip = "Remove the matching serialized configuration line."),
	Clear	UMETA(DisplayName = "Clear", ToolTip = "Clear all values for the configuration key with Unreal's exclamation prefix.")
};

/**
 * Selects the condition that activates a configuration action.
 *
 * @author Nils Bergemann
 */
UENUM(BlueprintType)
enum class EGorgeousConfigTrigger : uint8
{
	OnFirstStart			UMETA(DisplayName = "On First Start", ToolTip = "Run the configuration action the first time the system starts."),
	IfNotPresent			UMETA(DisplayName = "If Not Present", ToolTip = "Run when the configuration key or value is not present."),
	IfPresent				UMETA(DisplayName = "If Present", ToolTip = "Run when the configuration key or value is present."),
	IfNotSetToValue		UMETA(DisplayName = "If Not Set To Value", ToolTip = "Run when the configuration key is not set to the requested value."),
	OnAssetOpened			UMETA(DisplayName = "On Asset Opened", ToolTip = "Run when one of the configured trigger assets is open in an editor.")
};

/**
 * Selects the condition that controls whether a prompt is shown.
 *
 * @author Nils Bergemann
 */
UENUM(BlueprintType)
enum class EGorgeousPromptCondition : uint8
{
	OnFirstStart				UMETA(DisplayName = "On First Start", ToolTip = "Show the prompt the first time the owning system starts."),
	IfActionPredictionSucceeds	UMETA(DisplayName = "If Action Prediction Succeeds", ToolTip = "Show the prompt when the instanced action predicts that it can execute."),
	IfActionPredictionFails		UMETA(DisplayName = "If Action Prediction Fails", ToolTip = "Show the prompt when the instanced action predicts that it cannot execute.")
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Manage Configuration
| Functional Name: UGorgeousManageConfigurationAction
| Parent Class: UGorgeousGeneralSystemAction
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Reads and modifies Unreal configuration values when their trigger is satisfied.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName = "Manage Configuration",
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/Overview/#general-system-configuration",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousManageConfigurationAction : public UGorgeousGeneralSystemAction
{
	GENERATED_BODY()
	friend class UGorgeousProjectSettingsContribution;
public:
	UGorgeousManageConfigurationAction() { Target = EGorgeousSystemActionTarget::Editor; }

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	// Implements the PredictAction_Implementation override.
	virtual bool PredictAction_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration) const override;

	// Implements the OnSystemActivated_Implementation override.
	virtual void OnSystemActivated_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration) override;

	// Implements the OnSystemDeactivated_Implementation override.
	virtual void OnSystemDeactivated_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration) override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	/**
	 * Supplies the Details panel with supported configuration targets.
	 *
	 * @return The supported configuration target names.
	 */
	UFUNCTION()
	TArray<FString> GetConfigurationTargets() const;
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:

	// Configuration files to inspect and modify. Game and GameplayTags use Game.ini.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (GetOptions = "GetConfigurationTargets"))
	FString ConfigurationTarget;

	// Config section that contains the key.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	FString Section;

	// Config key to add, remove, clear, or test.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	FString Key;

	// Config value written for the selected key.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	FString Value;

	// Operation applied to the selected configuration key.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	EGorgeousConfigOperation Operation = EGorgeousConfigOperation::NewLine;

	// Condition that determines when the configuration entry is registered.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	EGorgeousConfigTrigger Trigger = EGorgeousConfigTrigger::IfNotPresent;

	// Shows a confirmation prompt before registering the configuration.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	bool bPrompt = false;

	// Makes the prompt informational and prevents declining the required configuration.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (EditCondition = "bPrompt"))
	bool bRequired = false;

	// Enables the minimum and maximum engine-version constraints.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	bool bVersionDependent = false;

	// Lowest engine version for which this configuration action is active.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (EditCondition = "bVersionDependent", EditConditionHides))
	FString MinEngineVersion;

	// Highest engine version for which this configuration action is active.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (EditCondition = "bVersionDependent", EditConditionHides))
	FString MaxEngineVersion;

	// Assets whose editor must be open when the trigger is OnAssetOpened.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (EditCondition = "Trigger == EGorgeousConfigTrigger::OnAssetOpened", EditConditionHides))
	TArray<TSoftObjectPtr<UObject>> TriggerAssets;

	// Message shown by the optional configuration prompt.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (MultiLine = true, EditCondition = "bPrompt", EditConditionHides))
	FText PromptMessage;

	// The value read during the last evaluation. Empty means the key was not found.
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Configuration")
	mutable FString ReadValue;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables


private:
	TArray<FString> ResolveConfigFiles() const;
	bool IsTriggerSatisfied() const;
	bool ApplyConfiguration(UGeneralSystemConfiguration_PDA* SystemConfiguration) const;
#if WITH_EDITOR
	void BindAssetOpenedEvent();
	void UnbindAssetOpenedEvent();
	void HandleAssetOpened(UObject* Asset, IAssetEditorInstance* EditorInstance);
	FDelegateHandle AssetOpenedHandle;
	bool bHandlingAssetOpened = false;
#endif
};

/**
 * Stores the result of evaluating a general system condition.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType)
struct FGorgeousSystemActionEvaluation
{
	GENERATED_BODY()

	// True when the condition matched its configured requirement.
	UPROPERTY(BlueprintReadOnly, Category = "Evaluation")
	bool bConditionMet = false;

	// Value produced by the condition, such as a config value or manifest JSON.
	UPROPERTY(BlueprintReadOnly, Category = "Evaluation")
	FString Value;

	// Human-readable explanation of the evaluation result.
	UPROPERTY(BlueprintReadOnly, Category = "Evaluation")
	FText Reason;
};

/**
 * Selects the visual style of a prompt option.
 *
 * @author Nils Bergemann
 */
UENUM(BlueprintType)
enum class EGorgeousPromptButtonStyle : uint8
{
	Primary UMETA(DisplayName = "Blue", ToolTip = "Primary corrective action."),
	Secondary UMETA(DisplayName = "Grey", ToolTip = "Secondary or ignore action."),
	Warning UMETA(DisplayName = "Yellow", ToolTip = "Action that requires extra attention."),
	Destructive UMETA(DisplayName = "Red", ToolTip = "Destructive action.")
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: General System Condition
| Functional Name: UGorgeousSystemCondition
| Parent Class: UObject
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Evaluates whether a general system condition is satisfied.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/Overview/#general-system-configuration",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousSystemCondition : public UObject
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	// Implements the Evaluate_Implementation override.
	virtual bool Evaluate_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration, FGorgeousSystemActionEvaluation& Evaluation) const;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	/**
	 * Evaluates this condition for the supplied system.
	 *
	 * @param SystemConfiguration The system being evaluated.
	 * @param Evaluation Receives the condition result and explanation.
	 * @return True when the condition evaluation completes successfully.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Gorgeous Core|Conditions")
	bool Evaluate(UGeneralSystemConfiguration_PDA* SystemConfiguration, FGorgeousSystemActionEvaluation& Evaluation) const;
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Configuration Value Condition
| Functional Name: UGorgeousConfigValueCondition
| Parent Class: UGorgeousSystemCondition
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Evaluates a configuration value against an optional expected value.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName = "Configuration Value Condition",
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/Overview/#general-system-configuration",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousConfigValueCondition : public UGorgeousSystemCondition
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	// Implements the Evaluate_Implementation override.
	virtual bool Evaluate_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration, FGorgeousSystemActionEvaluation& Evaluation) const override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:

	/**
	 * Supplies the Details panel with supported configuration targets.
	 *
	 * @return The supported configuration target names.
	 */
	UFUNCTION()
	TArray<FString> GetConfigurationTargets() const;
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:

	// Configuration target or ini filename to inspect.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (GetOptions = "GetConfigurationTargets"))
	FString ConfigurationTarget;

	// Config section containing the value.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	FString Section;

	// Config key to inspect.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	FString Key;

	// Requires the current value to equal ExpectedValue.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	bool bUseExpectedValue = true;

	// Optional value that must match when bUseExpectedValue is enabled.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (EditCondition = "bUseExpectedValue", EditConditionHides))
	FString ExpectedValue;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Class In Configuration Condition
| Functional Name: UGorgeousClassInheritanceCondition
| Parent Class: UGorgeousConfigValueCondition
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Evaluates whether a configured class matches or derives from an expected class.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName = "Class In Configuration Condition",
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/Overview/#general-system-configuration",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousClassInheritanceCondition : public UGorgeousConfigValueCondition
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	// Implements the Evaluate_Implementation override.
	virtual bool Evaluate_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration, FGorgeousSystemActionEvaluation& Evaluation) const override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables

	// Class that the configured class must match or derive from.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
	TSoftClassPtr<UObject> ExpectedClass;

	// Controls whether exact equality or inheritance is accepted.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
	EGorgeousClassComparison Comparison = EGorgeousClassComparison::ClassOrSubclass;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Asset Exists Condition
| Functional Name: UGorgeousAssetExistsCondition
| Parent Class: UGorgeousSystemCondition
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Evaluates whether a configured asset can be found.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName = "Asset Exists Condition",
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/Overview/#general-system-configuration",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousAssetExistsCondition : public UGorgeousSystemCondition
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	// Implements the Evaluate_Implementation override.
	virtual bool Evaluate_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration, FGorgeousSystemActionEvaluation& Evaluation) const override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables

	// Asset whose existence should be checked.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
	TSoftObjectPtr<UObject> Asset;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Plugin Enabled Condition
| Functional Name: UGorgeousPluginEnabledCondition
| Parent Class: UGorgeousSystemCondition
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Evaluates whether a named plugin is enabled.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName = "Plugin Enabled Condition",
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/Overview/#general-system-configuration",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousPluginEnabledCondition : public UGorgeousSystemCondition
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	// Implements the Evaluate_Implementation override.
	virtual bool Evaluate_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration, FGorgeousSystemActionEvaluation& Evaluation) const override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables

	// Plugin name to check.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Plugin")
	FName PluginName;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: System Present Condition
| Functional Name: UGorgeousSystemPresentCondition
| Parent Class: UGorgeousSystemCondition
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Evaluates whether a general system and optional manifest are available.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName = "System Present Condition",
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/Overview/#general-system-configuration",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousSystemPresentCondition : public UGorgeousSystemCondition
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	// Implements the Evaluate_Implementation override; Returns the raw SystemManifest.json in the evaluation value when found.
	virtual bool Evaluate_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration, FGorgeousSystemActionEvaluation& Evaluation) const override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables

	// General system PDA to locate and inspect.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System")
	TSoftObjectPtr<UGeneralSystemConfiguration_PDA> System;

	// Requires a readable SystemManifest.json in addition to the PDA.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System")
	bool bRequireManifest = false;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Action Prediction Condition
| Functional Name: UGorgeousActionPredictionCondition
| Parent Class: UGorgeousSystemCondition
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Evaluates an instanced action and compares its predicted result.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName = "Action Prediction Condition",
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/Overview/#general-system-configuration",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousActionPredictionCondition : public UGorgeousSystemCondition
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	// Implements the Evaluate_Implementation override.
	virtual bool Evaluate_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration, FGorgeousSystemActionEvaluation& Evaluation) const override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables

	// Instanced action whose prediction should be evaluated.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Action")
	TObjectPtr<UGorgeousGeneralSystemAction> Action;

	// Expected result of the action prediction.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	bool bExpectedResult = true;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Prompt Option
| Functional Name: UGorgeousPromptOption
| Parent Class: UObject
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Defines an action and presentation style for one prompt option.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/Overview/#general-system-configuration",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousPromptOption : public UObject
{
	GENERATED_BODY()

	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:

	// Text shown on the prompt button.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prompt")
	FText Label;

	// Visual severity/style of the prompt button.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prompt")
	EGorgeousPromptButtonStyle Style = EGorgeousPromptButtonStyle::Primary;

	// Action executed when this option is selected.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Prompt")
	TObjectPtr<UGorgeousGeneralSystemAction> Action;

	// Persists this prompt as dismissed after selecting the option.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prompt")
	bool bDismissPrompt = false;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Reparent Blueprint
| Functional Name: UGorgeousReparentBlueprintAction
| Parent Class: UGorgeousGeneralSystemAction
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Changes the parent class of a configured Blueprint asset.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName = "Reparent Blueprint",
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/Overview/#general-system-configuration",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousReparentBlueprintAction : public UGorgeousGeneralSystemAction
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	// Implements the PredictAction_Implementation override.
	virtual bool PredictAction_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration) const override;

	// Implements the ExecuteAction_Implementation override.
	virtual bool ExecuteAction_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration) override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables

	// When enabled, the Blueprint asset path is read from Manage Configuration.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blueprint")
	bool bDrawBlueprintFromConfig = false;

	// Blueprint asset whose parent class should be changed.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blueprint", meta = (EditCondition = "!bDrawBlueprintFromConfig", EditConditionHides))
	TSoftObjectPtr<UBlueprint> Blueprint;

	// Configuration action that reads the Blueprint asset path when config-driven mode is enabled.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Blueprint", meta = (EditCondition = "bDrawBlueprintFromConfig", EditConditionHides))
	TObjectPtr<UGorgeousManageConfigurationAction> BlueprintConfig;

	// New parent class for the Blueprint.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blueprint")
	TSoftClassPtr<UObject> NewParentClass;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Trigger Prompt
| Functional Name: UGorgeousTriggerPromptAction
| Parent Class: UGorgeousGeneralSystemAction
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Shows an editor prompt based on a reusable condition or action prediction.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName = "Trigger Prompt",
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/Overview/#general-system-configuration",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/General/Overview/"
		))
class GORGEOUSCORERUNTIME_API UGorgeousTriggerPromptAction : public UGorgeousGeneralSystemAction
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:

	// Implements the PredictAction_Implementation override.
	virtual bool PredictAction_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration) const override;

	// Implements the OnSystemActivated_Implementation override.
	virtual void OnSystemActivated_Implementation(UGeneralSystemConfiguration_PDA* SystemConfiguration) override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables

	// Determines when this prompt is shown when no reusable condition object is configured.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prompt", meta = (EditCondition = "ConditionObject == nullptr", EditConditionHides))
	EGorgeousPromptCondition Condition = EGorgeousPromptCondition::OnFirstStart;

	// Inverts the condition result, allowing prompts to target invalid or missing states.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prompt")
	bool bInvertCondition = false;

	// Reusable condition evaluated before showing the prompt.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Prompt")
	TObjectPtr<UGorgeousSystemCondition> ConditionObject;

	// Instanced action whose prediction result controls this prompt when no reusable condition object is configured.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Prompt", meta = (EditCondition = "ConditionObject == nullptr && Condition != EGorgeousPromptCondition::OnFirstStart", EditConditionHides))
	TObjectPtr<UGorgeousGeneralSystemAction> ActionToPredict;

	// Message displayed in the editor prompt.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prompt", meta = (MultiLine = true))
	FText Message;

	// Actions offered to the developer as selectable prompt buttons.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Prompt")
	TArray<TObjectPtr<UGorgeousPromptOption>> Options;

	// Result from the most recent condition evaluation, including manifest JSON when available.
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Prompt")
	mutable FGorgeousSystemActionEvaluation LastEvaluation;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};
