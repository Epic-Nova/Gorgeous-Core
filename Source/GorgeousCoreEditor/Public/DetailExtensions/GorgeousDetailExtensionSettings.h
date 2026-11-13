// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once
#include "Engine/DeveloperSettings.h"
#include "GorgeousDetailExtensionSettings.generated.h"

UCLASS(Config=EditorPerProjectUserSettings, DisplayName="Core Detail Extensions")
class GORGEOUSCOREEDITOR_API UGorgeousDetailExtensionSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Config, Category="Detail Extensions")
	bool bEnableDetailExtensions = true;

	/** Registered extension identifiers that should use the normal property editor instead. */
	UPROPERTY(EditAnywhere, Config, Category="Detail Extensions")
	TSet<FName> DisabledExtensions;

#if WITH_EDITOR
	virtual FName GetCategoryName() const override { return TEXT("Gorgeous Things"); }
	virtual void PostEditChangeProperty(FPropertyChangedEvent& Event) override;
#endif
};
