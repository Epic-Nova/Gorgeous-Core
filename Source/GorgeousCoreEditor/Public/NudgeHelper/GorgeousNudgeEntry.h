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
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousNudgeEntry.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Enumeration defining the audience scope for a nudge.
 *
 * @author Nils Bergemann
 */
UENUM(BlueprintType, meta = (ShortTooltip = "Defines which installed Gorgeous content receives a nudge."))
enum class EGorgeousNudgeScope : uint8
{
	PluginUpdate UMETA(DisplayName = "Plugin Update", ToolTip = "Targets an installed Gorgeous plugin."),
	GeneralSystemUpdate UMETA(DisplayName = "General System Update", ToolTip = "Targets an installed Gorgeous general system."),
	EcosystemNews UMETA(DisplayName = "Ecosystem News", ToolTip = "Targets every Gorgeous ecosystem installation."),
	NewPluginTutorial UMETA(DisplayName = "New Plugin Tutorial", ToolTip = "Targets an installed plugin with a tutorial recommendation.")
};

/**
 * Structure describing an actionable update, news item, or tutorial recommendation.
 *
 * @author Nils Bergemann
 */
USTRUCT(BlueprintType, meta = (
	DocumentationOverview = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/NudgeHelper/Overview",
	DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/NudgeHelper/GorgeousNudgeEntry",
	DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Editor/NudgeHelper/Examples"
	))
struct GORGEOUSCOREEDITOR_API FGorgeousNudgeEntry
{
	GENERATED_BODY()

	// Identifies this nudge across editor sessions.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nudge")
	FString Id;

	// Defines the content that receives this nudge.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nudge")
	EGorgeousNudgeScope Scope = EGorgeousNudgeScope::EcosystemNews;

	// Identifies the targeted plugin or general system.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nudge")
	FString TargetId;

	// Identifies the version represented by this nudge.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nudge")
	FString Version;

	// Names the nudge in the carousel.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nudge")
	FText Title;

	// Explains the change or recommendation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nudge", meta = (MultiLine = true))
	FText Description;

	// Provides the complete HTML document rendered for this nudge.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nudge", meta = (MultiLine = true))
	FString HtmlPage;

	// Provides Markdown content rendered locally by the carousel when present.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nudge", meta = (MultiLine = true))
	FString MarkdownContent;

	// Identifies a Markdown document that will be downloaded before the nudge is shown.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nudge")
	FString MarkdownUrl;

	// Requests a carousel size for this Markdown entry. A zero component uses the default size.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nudge")
	FVector2D PreferredWindowSize = FVector2D::ZeroVector;

	// Labels the action shown by the carousel.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nudge")
	FString ActionLabel;

	// Opens the destination associated with the nudge.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nudge")
	FString ActionUrl;

	// Uses the existing Gorgeous plugin installer rather than opening ActionUrl.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nudge")
	bool bUsePluginDownloader = false;

	// Carries the short-lived installer token supplied by the news endpoint when needed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nudge")
	FString PluginDownloadToken;
};
