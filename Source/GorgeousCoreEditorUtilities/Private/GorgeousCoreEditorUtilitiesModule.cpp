// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|        that has nothing in common with Epic Games in any capacity.        |
<==========================================================================*/
#include "GorgeousCoreEditorUtilitiesModule.h"
#include "AssetRegistration/GorgeousAssetTypeAction.h"
#include "AssetRegistration/GorgeousCoreBlueprintTypes.h"
#include "ObjectVariables/GorgeousObjectVariable.h"
#include "MessageLogModule.h"
#include "Logging/TokenizedMessage.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Editor.h"
#include "InsightMatrix/GorgeousInsightMatrixSubsystem.h"
#include "GorgeousLoggingDeveloperSettings.h"
#include "Logging/MessageLog.h"
#include "Styling/CoreStyle.h"
#include "ToolMenus.h"
#include "AssetToolsModule.h"
#include "AssetTypeCategories.h"
#include "GPUMessaging.h"
#include "IAssetTools.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Brushes/SlateImageBrush.h"
#include "Styling/AppStyle.h"
#include "Misc/Paths.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
//<--------------------------=== Module Includes ===------------------------->
//<-------------------------------------------------------------------------->

//=============================================================================
// FGorgeousCoreEditorUtilitiesModule Implementation
//=============================================================================

#define LOCTEXT_NAMESPACE "GorgeousCoreEditorUtilities"

namespace GorgeousEditorLogging
{
	void ExecuteLogHyperlinkAction(const FGorgeousLogHyperlink& Hyperlink);
}

static EMessageSeverity::Type ToMessageSeverity(const EGorgeousLoggingImportance Importance)
{
	switch (Importance)
	{
		case Logging_Warning:
			return EMessageSeverity::Warning;
		case Logging_Error:
		case Logging_Fatal:
			return EMessageSeverity::Error;
		case Logging_Success:
		case Logging_Information:
		default:
			return EMessageSeverity::Info;
	}
}

static void OpenAssetInEditor(const FSoftObjectPath& AssetPath)
{
	if (!AssetPath.IsValid())
		return;

	UObject* LoadedAsset = AssetPath.TryLoad();
	if (!LoadedAsset || !GEditor)
		return;

	if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
	{
		AssetEditorSubsystem->OpenEditorForAsset(LoadedAsset);
	}
}

static void ShowMissingAssetNotification()
{
	FNotificationInfo Info(LOCTEXT("MissingAssetNotification", "Asset registry entry missing."));
	Info.ExpireDuration = 5.0f;
	Info.bUseLargeFont = false;

	FSlateNotificationManager::Get().AddNotification(Info);
}

static void EmitGorgeousLogEntry(const FGorgeousLogEntry& Entry, bool bForce)
{
	const UGorgeousLoggingDeveloperSettings* Settings = GetDefault<UGorgeousLoggingDeveloperSettings>();
	if (!Settings)
	{
		return;
	}

	if (!Settings->bEnableGorgeousMessageLog && !bForce)
	{
		return;
	}

	auto ExecuteHyperlink = [Entry]()
	{
		if (!Entry.Hyperlink.IsValid())
		{
			ShowMissingAssetNotification();
			return;
		}

		if (!Entry.Hyperlink.ActionName.IsNone())
		{
			GorgeousEditorLogging::ExecuteLogHyperlinkAction(Entry.Hyperlink);
			return;
		}

		if (Entry.Hyperlink.TargetAsset.IsValid())
		{
			UObject* Asset = Entry.Hyperlink.TargetAsset.ResolveObject();
			if (!Asset)
			{
				Asset = Entry.Hyperlink.TargetAsset.TryLoad();
			}
			if (Asset)
			{
				OpenAssetInEditor(Entry.Hyperlink.TargetAsset);
				return;
			}
		}

		ShowMissingAssetNotification();
	};

	FMessageLog MessageLog(Settings->MessageLogListingName);
	TSharedRef<FTokenizedMessage> Tokenized = FTokenizedMessage::Create(ToMessageSeverity(Entry.Importance));
	Tokenized->AddToken(FTextToken::Create(FText::FromString(Entry.Message)));

	if (Entry.Hyperlink.IsValid())
	{
		Tokenized->AddToken(FActionToken::Create(
			FText::FromString(Entry.Hyperlink.LinkText),
			FText::FromString(Entry.Message),
			FOnActionTokenExecuted::CreateLambda(ExecuteHyperlink)
		));

		FNotificationInfo Info(FText::FromString(Entry.Message));
		Info.bUseThrobber = false;
		Info.bUseLargeFont = false;
		Info.ExpireDuration = Entry.Duration > 0.0f ? Entry.Duration : 5.0f;
		Info.HyperlinkText = FText::FromString(Entry.Hyperlink.LinkText);
		Info.Hyperlink = FSimpleDelegate::CreateLambda(ExecuteHyperlink);
		FSlateNotificationManager::Get().AddNotification(Info);
	}

	MessageLog.AddMessage(Tokenized);
}

namespace
{
	FToolMenuOwnerScoped GDebugMenuOwner{TEXT("GorgeousCoreEditorUtilities")};
	TArray<TSharedPtr<IAssetTypeActions>> GRegisteredAssetTypeActions;
	EAssetTypeCategories::Type GGorgeousThingsCategory = EAssetTypeCategories::Misc;
	TSharedPtr<FSlateStyleSet> GGorgeousCoreEditorStyle;

	void RegisterGorgeousCoreEditorStyle()
	{
		if (GGorgeousCoreEditorStyle.IsValid())
		{
			return;
		}

		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("GorgeousCore"));
		if (!Plugin.IsValid())
		{
			return;
		}

		GGorgeousCoreEditorStyle = MakeShared<FSlateStyleSet>(TEXT("GorgeousCoreEditorStyle"));
		GGorgeousCoreEditorStyle->SetContentRoot(Plugin->GetBaseDir() / TEXT("Resources"));

		const FVector2D Icon16(16.0f, 16.0f);
		const FVector2D Thumb128(128.0f, 128.0f);

		auto SetBrushes = [&](const FString& Key, const FString& FileName, const FString& ClassName)
		{
			GGorgeousCoreEditorStyle->Set(*FString::Printf(TEXT("GorgeousCore.%s.Icon"), *Key),
				new FSlateImageBrush(GGorgeousCoreEditorStyle->RootToContentDir(FileName, TEXT(".png")), Icon16));
			GGorgeousCoreEditorStyle->Set(*FString::Printf(TEXT("GorgeousCore.%s.Thumbnail"), *Key),
				new FSlateImageBrush(GGorgeousCoreEditorStyle->RootToContentDir(FileName, TEXT(".png")), Thumb128));

			GGorgeousCoreEditorStyle->Set(*FString::Printf(TEXT("ClassIcon.%s"), *ClassName),
				new FSlateImageBrush(GGorgeousCoreEditorStyle->RootToContentDir(FileName, TEXT(".png")), Icon16));
			GGorgeousCoreEditorStyle->Set(*FString::Printf(TEXT("ClassThumbnail.%s"), *ClassName),
				new FSlateImageBrush(GGorgeousCoreEditorStyle->RootToContentDir(FileName, TEXT(".png")), Thumb128));
		};

		auto SetNativeClassBrushes = [&](const FString& FileName, const FString& NativeClassName)
		{
			GGorgeousCoreEditorStyle->Set(*FString::Printf(TEXT("ClassIcon.%s"), *NativeClassName),
				new FSlateImageBrush(GGorgeousCoreEditorStyle->RootToContentDir(FileName, TEXT(".png")), Icon16));
			GGorgeousCoreEditorStyle->Set(*FString::Printf(TEXT("ClassThumbnail.%s"), *NativeClassName),
				new FSlateImageBrush(GGorgeousCoreEditorStyle->RootToContentDir(FileName, TEXT(".png")), Thumb128));
		};

		auto MapParentClassBrushes = [&](const FString& ClassName, const FString& ParentClassName)
		{
			GGorgeousCoreEditorStyle->Set(*FString::Printf(TEXT("ClassIcon.%s"), *ClassName),
				const_cast<FSlateBrush*>(FAppStyle::Get().GetBrush(*FString::Printf(TEXT("ClassIcon.%s"), *ParentClassName))));
			GGorgeousCoreEditorStyle->Set(*FString::Printf(TEXT("ClassThumbnail.%s"), *ClassName),
				const_cast<FSlateBrush*>(FAppStyle::Get().GetBrush(*FString::Printf(TEXT("ClassThumbnail.%s"), *ParentClassName))));
		};

		auto MapEngineSvgBrushes = [&](const FString& ClassName, const FString& RelativeSvgPath)
		{
			const FString SvgPath = FPaths::Combine(FPaths::EngineContentDir(), TEXT("Editor/Slate/Starship"), RelativeSvgPath);
			GGorgeousCoreEditorStyle->Set(*FString::Printf(TEXT("ClassIcon.%s"), *ClassName),
				new FSlateVectorImageBrush(SvgPath, Icon16));
			GGorgeousCoreEditorStyle->Set(*FString::Printf(TEXT("ClassThumbnail.%s"), *ClassName),
				new FSlateVectorImageBrush(SvgPath, Thumb128));
		};

		SetBrushes(TEXT("ObjectVariable"), TEXT("variable-cube"), TEXT("GorgeousObjectVariableBlueprint"));
		SetBrushes(TEXT("ConditionalObjectChooser"), TEXT("chooser-funnel"), TEXT("GorgeousConditionalObjectChooserBlueprint"));
		SetBrushes(TEXT("Condition"), TEXT("condition"), TEXT("GorgeousConditionBlueprint"));

		SetNativeClassBrushes(TEXT("variable-cube"), TEXT("GorgeousObjectVariable"));
		SetNativeClassBrushes(TEXT("chooser-funnel"), TEXT("GorgeousConditionalObjectChooser"));
		SetNativeClassBrushes(TEXT("condition"), TEXT("GorgeousCondition"));

		MapParentClassBrushes(TEXT("GorgeousGameInstanceBlueprint"), TEXT("Object"));
		MapParentClassBrushes(TEXT("GorgeousGameModeBlueprint"), TEXT("GameModeBase"));
		MapParentClassBrushes(TEXT("GorgeousGameStateBlueprint"), TEXT("GameStateBase"));
		MapParentClassBrushes(TEXT("GorgeousPlayerControllerBlueprint"), TEXT("PlayerController"));
		MapEngineSvgBrushes(TEXT("GorgeousPlayerStateBlueprint"), TEXT("AssetIcons/Actor_64.svg"));
		MapEngineSvgBrushes(TEXT("GorgeousWorldSettingsBlueprint"), TEXT("Common/WorldSettings.svg"));

		FSlateStyleRegistry::RegisterSlateStyle(*GGorgeousCoreEditorStyle);
	}

	void UnregisterGorgeousCoreEditorStyle()
	{
		if (!GGorgeousCoreEditorStyle.IsValid())
		{
			return;
		}

		FSlateStyleRegistry::UnRegisterSlateStyle(*GGorgeousCoreEditorStyle);
		GGorgeousCoreEditorStyle.Reset();
	}

	void RegisterAssetTypeAction(IAssetTools& AssetTools, const FGorgeousAssetTypeActionInfo_S& Info)
	{
		TSharedRef<IAssetTypeActions> Action = MakeShared<FGorgeousAssetTypeAction>(Info);
		AssetTools.RegisterAssetTypeActions(Action);
		GRegisteredAssetTypeActions.Add(Action);
	}

	void RegisterGorgeousAssetTypeActions()
	{
		RegisterGorgeousCoreEditorStyle();

		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		GGorgeousThingsCategory = AssetTools.RegisterAdvancedAssetCategory(
			"GorgeousThings",
			NSLOCTEXT("GorgeousCore", "Category_GorgeousThings", "Gorgeous Things"));

		const FText CoreMenu = NSLOCTEXT("GorgeousCore", "Menu_GorgeousCore", "Gorgeous Core");
		const FText Menu_ConditionalChoosers = NSLOCTEXT("GorgeousCore", "Menu_ConditionalChoosers", "Conditional Object Choosers");
		const FText Menu_Conditions = NSLOCTEXT("GorgeousCore", "Menu_Conditions", "Conditions");
		const FText Menu_ObjectVariables = NSLOCTEXT("GorgeousCore", "Menu_ObjectVariables", "Object Variables");
		const FText Menu_QualityOfLife = NSLOCTEXT("GorgeousCore", "Menu_QualityOfLife", "Quality of Life");
		
		auto MakeInfo = [&](const FText& DisplayName, UClass* SupportedClass, const FColor& TypeColor, TArray<FText> SubMenus,
			const FName& IconKey)
		{
			FGorgeousAssetTypeActionInfo_S Info;
			Info.DisplayName = DisplayName;
			Info.SupportedClass = SupportedClass;
			Info.TypeColor = TypeColor;
			Info.Categories = GGorgeousThingsCategory;
			Info.SubMenus = MoveTemp(SubMenus);
			if (GGorgeousCoreEditorStyle.IsValid() && !IconKey.IsNone())
			{
				Info.ThumbnailBrush = GGorgeousCoreEditorStyle->GetBrush(*FString::Printf(TEXT("GorgeousCore.%s.Thumbnail"), *IconKey.ToString()));
				Info.IconBrush = GGorgeousCoreEditorStyle->GetBrush(*FString::Printf(TEXT("GorgeousCore.%s.Icon"), *IconKey.ToString()));
			}
			else
			{
				Info.ThumbnailBrush = nullptr;
				Info.IconBrush = nullptr;
			}
			return Info;
		};

		auto MakeInfoWithBrushes = [&](const FText& DisplayName, UClass* SupportedClass, const FColor& TypeColor, TArray<FText> SubMenus,
			const FSlateBrush* IconBrush, const FSlateBrush* ThumbnailBrush)
		{
			FGorgeousAssetTypeActionInfo_S Info;
			Info.DisplayName = DisplayName;
			Info.SupportedClass = SupportedClass;
			Info.TypeColor = TypeColor;
			Info.Categories = GGorgeousThingsCategory;
			Info.SubMenus = MoveTemp(SubMenus);
			Info.IconBrush = IconBrush;
			Info.ThumbnailBrush = ThumbnailBrush;
			return Info;
		};

		RegisterAssetTypeAction(AssetTools, MakeInfo(
			NSLOCTEXT("GorgeousCore", "ObjectVariable", "Gorgeous Object Variable"),
			UGorgeousObjectVariableBlueprint::StaticClass(),
			FColor::Blue,
			{ CoreMenu },
			TEXT("ObjectVariable")));

		RegisterAssetTypeAction(AssetTools, MakeInfo(
			NSLOCTEXT("GorgeousCore", "ConditionalObjectChooser", "Gorgeous Conditional Object Chooser"),
			UGorgeousConditionalObjectChooserBlueprint::StaticClass(),
			FColor::Cyan,
			{ CoreMenu, Menu_ConditionalChoosers },
			TEXT("ConditionalObjectChooser")));

		RegisterAssetTypeAction(AssetTools, MakeInfo(
			NSLOCTEXT("GorgeousCore", "Condition", "Gorgeous Condition"),
			UGorgeousConditionBlueprint::StaticClass(),
			FColor::Turquoise,
			{ CoreMenu, Menu_ConditionalChoosers, Menu_Conditions },
			TEXT("Condition")));

		RegisterAssetTypeAction(AssetTools, MakeInfo(
			NSLOCTEXT("GorgeousCore", "GameInstance", "Gorgeous Game Instance"),
			UGorgeousGameInstanceBlueprint::StaticClass(),
			FColor::Blue,
			{ CoreMenu, Menu_QualityOfLife },
			NAME_None));

		RegisterAssetTypeAction(AssetTools, MakeInfoWithBrushes(
			NSLOCTEXT("GorgeousCore", "GameMode", "Gorgeous Game Mode"),
			UGorgeousGameModeBlueprint::StaticClass(),
			FColor::Blue,
			{ CoreMenu, Menu_QualityOfLife },
			FAppStyle::Get().GetBrush(TEXT("ClassIcon.GameModeBase")),
			FAppStyle::Get().GetBrush(TEXT("ClassThumbnail.GameModeBase"))));

		RegisterAssetTypeAction(AssetTools, MakeInfoWithBrushes(
			NSLOCTEXT("GorgeousCore", "GameState", "Gorgeous Game State"),
			UGorgeousGameStateBlueprint::StaticClass(),
			FColor::Blue,
			{ CoreMenu, Menu_QualityOfLife },
			FAppStyle::Get().GetBrush(TEXT("ClassIcon.GameStateBase")),
			FAppStyle::Get().GetBrush(TEXT("ClassThumbnail.GameStateBase"))));

		RegisterAssetTypeAction(AssetTools, MakeInfoWithBrushes(
			NSLOCTEXT("GorgeousCore", "PlayerController", "Gorgeous Player Controller"),
			UGorgeousPlayerControllerBlueprint::StaticClass(),
			FColor::Blue,
			{ CoreMenu, Menu_QualityOfLife },
			FAppStyle::Get().GetBrush(TEXT("ClassIcon.PlayerController")),
			FAppStyle::Get().GetBrush(TEXT("ClassThumbnail.PlayerController"))));

		RegisterAssetTypeAction(AssetTools, MakeInfoWithBrushes(
			NSLOCTEXT("GorgeousCore", "PlayerState", "Gorgeous Player State"),
			UGorgeousPlayerStateBlueprint::StaticClass(),
			FColor::Blue,
			{ CoreMenu, Menu_QualityOfLife },
			GGorgeousCoreEditorStyle.IsValid() ? GGorgeousCoreEditorStyle->GetBrush(TEXT("ClassIcon.GorgeousPlayerStateBlueprint")) : nullptr,
			GGorgeousCoreEditorStyle.IsValid() ? GGorgeousCoreEditorStyle->GetBrush(TEXT("ClassThumbnail.GorgeousPlayerStateBlueprint")) : nullptr));

		RegisterAssetTypeAction(AssetTools, MakeInfoWithBrushes(
			NSLOCTEXT("GorgeousCore", "WorldSettings", "Gorgeous World Settings"),
			UGorgeousWorldSettingsBlueprint::StaticClass(),
			FColor::Blue,
			{ CoreMenu, Menu_QualityOfLife },
			GGorgeousCoreEditorStyle.IsValid() ? GGorgeousCoreEditorStyle->GetBrush(TEXT("ClassIcon.GorgeousWorldSettingsBlueprint")) : nullptr,
			GGorgeousCoreEditorStyle.IsValid() ? GGorgeousCoreEditorStyle->GetBrush(TEXT("ClassThumbnail.GorgeousWorldSettingsBlueprint")) : nullptr));
	}

	void UnregisterGorgeousAssetTypeActions()
	{
		if (!FModuleManager::Get().IsModuleLoaded("AssetTools"))
		{
			GRegisteredAssetTypeActions.Reset();
			return;
		}

		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (const TSharedPtr<IAssetTypeActions>& Action : GRegisteredAssetTypeActions)
		{
			if (Action.IsValid())
			{
				AssetTools.UnregisterAssetTypeActions(Action.ToSharedRef());
			}
		}
		GRegisteredAssetTypeActions.Reset();
		UnregisterGorgeousCoreEditorStyle();
	}

	void RegisterDebugMenuEntry()
	{
		if (!UToolMenus::IsToolMenuUIEnabled())
		{
			return;
		}

		UToolMenus* ToolMenus = UToolMenus::Get();
		if (!ToolMenus)
		{
			return;
		}

		UToolMenu* AuditMenu = ToolMenus->ExtendMenu("LevelEditor.MainMenu.Tools.Audit");
		if (!AuditMenu)
		{
			return;
		}

		FToolMenuSection& AuditSection = AuditMenu->FindOrAddSection("GorgeousCoreAudit");
		AuditSection.AddMenuEntry(
			"GorgeousCore_ShowDebugPanel",
			NSLOCTEXT("GorgeousCoreAudit", "ShowDebugPanel", "Gorgeous Insight Matrix"),
			NSLOCTEXT("GorgeousCoreAudit", "ShowDebugPanel_Tooltip", "Open the Gorgeous Insight Matrix debug panel."),
			FSlateIcon(FCoreStyle::Get().GetStyleSetName(), "Icons.Info"),
			FUIAction(
				FExecuteAction::CreateLambda([]
				{
					if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
					{
						Subsystem->ShowDebugPanel();
					}
				}),
				FCanExecuteAction::CreateLambda([]
				{
					return UGorgeousInsightMatrixSubsystem::Get() != nullptr;
				})));
	}

	void UnregisterDebugMenuEntry()
	{
		if (!UToolMenus::IsToolMenuUIEnabled())
		{
			return;
		}

		UToolMenus* ToolMenus = UToolMenus::Get();
		if (!ToolMenus)
		{
			return;
		}

		ToolMenus->UnregisterOwner(GDebugMenuOwner.GetOwner());
	}
}

void FGorgeousCoreEditorUtilitiesModule::GorgeousStartupModule()
{
	const UGorgeousLoggingDeveloperSettings* Settings = GetDefault<UGorgeousLoggingDeveloperSettings>();
	if (Settings && (Settings->bEnableGorgeousMessageLog))
	{
		FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
		FMessageLogInitializationOptions Options;
		Options.bShowInLogWindow = true;
		Options.bAllowClear = true;
		Options.bScrollToBottom = true;
		MessageLogModule.RegisterLogListing(Settings->MessageLogListingName, LOCTEXT("GorgeousThingsLog", "Gorgeous Things"), Options);
		RegisteredLogListingName = Settings->MessageLogListingName;
	}

	RegisterGorgeousAssetTypeActions();

	LogEntryHandle = GetGorgeousLogEntryDelegate().AddRaw(this, &FGorgeousCoreEditorUtilitiesModule::HandleGorgeousLogEntry);
	BeginPIEHandle = FEditorDelegates::BeginPIE.AddRaw(this, &FGorgeousCoreEditorUtilitiesModule::HandleBeginPIE);
	EndPIEHandle = FEditorDelegates::EndPIE.AddRaw(this, &FGorgeousCoreEditorUtilitiesModule::HandleEndPIE);
	RegisterDebugMenuEntry();
}

void FGorgeousCoreEditorUtilitiesModule::GorgeousShutdownModule()
{
	if (BeginPIEHandle.IsValid())
	{
		FEditorDelegates::BeginPIE.Remove(BeginPIEHandle);
		BeginPIEHandle.Reset();
	}

	if (EndPIEHandle.IsValid())
	{
		FEditorDelegates::EndPIE.Remove(EndPIEHandle);
		EndPIEHandle.Reset();
	}

	UnregisterGorgeousAssetTypeActions();

	if (LogEntryHandle.IsValid())
	{
		GetGorgeousLogEntryDelegate().Remove(LogEntryHandle);
		LogEntryHandle.Reset();
	}

	UnregisterDebugMenuEntry();

	if (!RegisteredLogListingName.IsNone() && FModuleManager::Get().IsModuleLoaded("MessageLog"))
	{
		if (FMessageLogModule* MessageLogModule = FModuleManager::GetModulePtr<FMessageLogModule>("MessageLog"))
		{
			MessageLogModule->UnregisterLogListing(RegisteredLogListingName);
		}
		RegisteredLogListingName = NAME_None;
	}
}

TArray<FName> FGorgeousCoreEditorUtilitiesModule::GetDependentPlugins() const
{
	return TArray<FName>();
}

int32 FGorgeousCoreEditorUtilitiesModule::GetMinimumRequiredCoreVersion() const
{
	return 100; // Version 1.0
}

void FGorgeousCoreEditorUtilitiesModule::HandleGorgeousLogEntry(const FGorgeousLogEntry& Entry)
{
	EmitGorgeousLogEntry(Entry, false);
}

void FGorgeousCoreEditorUtilitiesModule::HandleBeginPIE(bool bIsSimulating)
{
	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		if (Subsystem->GetQueuedTestCount() > 0)
		{
			Subsystem->RunQueuedTests();
			Subsystem->RefreshDebugPanel();
		}
	}
}

void FGorgeousCoreEditorUtilitiesModule::HandleEndPIE(bool bIsSimulating)
{
	if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
	{
		Subsystem->RefreshDebugPanel();
	}
}

IMPLEMENT_MODULE(FGorgeousCoreEditorUtilitiesModule, GorgeousCoreEditorUtilities)

#undef LOCTEXT_NAMESPACE
