#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GorgeousLibrarySettings.generated.h"

/**
 * Settings for the Gorgeous Library and Core Systems.
 */
UCLASS(config=EditorPerProjectUserSettings)
class GORGEOUSCOREEDITOR_API UGorgeousLibrarySettings : public UObject
{
	GENERATED_BODY()

public:
	UGorgeousLibrarySettings();

	/** Interval (in editor startups) at which the system validation will be automatically executed. */
	UPROPERTY(EditAnywhere, Category = "Validation", meta = (ClampMin = "1"))
	int32 ValidationInterval;

	/** Number of times the system validation has been executed. */
	UPROPERTY(VisibleAnywhere, Category = "Validation")
	int32 ValidationCount;

	/** Manually triggers the Gorgeous Installer to recompile the project. The editor will prompt to save and then close gracefully. */
	UFUNCTION(CallInEditor, Category = "Recompilation")
	void TriggerRecompilation();

	/** Manually triggers a check against the API for new Gorgeous plugin updates. */
	UFUNCTION(CallInEditor, Category = "Updates")
	void CheckForUpdates();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostInitProperties() override;
#endif
};
