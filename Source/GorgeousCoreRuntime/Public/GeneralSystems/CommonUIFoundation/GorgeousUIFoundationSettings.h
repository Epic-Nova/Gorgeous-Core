// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GorgeousUIFoundationSettings.generated.h"

class UGorgeousPrimaryGameLayout;
class UGorgeousInputBinding_DA;

/**
 * Global settings for the Gorgeous UI Foundation.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Core ↪ UI Foundation"))
class GORGEOUSCORERUNTIME_API UGorgeousUIFoundationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGorgeousUIFoundationSettings(const FObjectInitializer& ObjectInitializer);

	/**
	 * The Blueprint class to use for the Primary Game Layout.
	 * This layout is automatically created for each local player.
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Layout", meta = (MetaClass = "/Script/GorgeousCoreRuntime.GorgeousPrimaryGameLayout"))
	TSoftClassPtr<UGorgeousPrimaryGameLayout> DefaultLayoutClass;
	
	/**
	 * Default input bindings to use if a specific UI state doesn't provide its own.
	 * Acts as a fallback for project-wide actions like "Back", "Confirm", etc.
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Input")
	TSoftObjectPtr<UGorgeousInputBinding_DA> DefaultInputBindings;

	/**
	 * Default theme to apply at startup if no other theme is active.
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Visuals")
	TSoftObjectPtr<class UGorgeousUITheme_DA> DefaultTheme;
	
#if WITH_EDITOR
	virtual FName GetCategoryName() const override { return TEXT("Gorgeous Things"); }
	virtual FName GetSectionName() const override { return TEXT("Core|UI Foundation"); }
	virtual FText GetSectionText() const override { return NSLOCTEXT("GorgeousCoreUIFoundationSettings", "SectionName", "Core ↪ UI Foundation"); }
	virtual FText GetSectionDescription() const override { return NSLOCTEXT("GorgeousCoreUIFoundationSettings", "SectionDescription", "Global settings for the Gorgeous UI Foundation, including default layout, theme, and input bindings."); }
#endif
};
