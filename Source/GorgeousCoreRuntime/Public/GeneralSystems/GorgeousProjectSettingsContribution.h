// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once
#include "GeneralSystems/GorgeousGeneralSystemAction.h"
#include "GorgeousProjectSettingsContribution.generated.h"

UENUM(BlueprintType)
enum class EGorgeousProjectSettingContribution : uint8
{
	AssetManagerSystems,
	DataRegistryDirectories,
	CommonUIViewport,
	InteractionTraceChannel
};

/** Explicit GSC-owned project setup. Validators only diagnose; these authored actions contribute settings. */
UCLASS(EditInlineNew, DefaultToInstanced, DisplayName="Contribute Project Settings")
class GORGEOUSCORERUNTIME_API UGorgeousProjectSettingsContribution : public UGorgeousGeneralSystemAction
{
	GENERATED_BODY()
public:
	UGorgeousProjectSettingsContribution() { Target = EGorgeousSystemActionTarget::Editor; }
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Project Settings")
	EGorgeousProjectSettingContribution Contribution = EGorgeousProjectSettingContribution::AssetManagerSystems;
	virtual void OnSystemActivated_Implementation(UGeneralSystemConfiguration_PDA* Configuration) override;
private:
	bool ApplyEntry(UGeneralSystemConfiguration_PDA* Configuration, const FString& TargetName, const FString& Section,
		const FString& Key, const FString& Value, EGorgeousConfigOperation Operation);
};
