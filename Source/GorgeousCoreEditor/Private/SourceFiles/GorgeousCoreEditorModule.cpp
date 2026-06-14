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
#include "GorgeousCoreEditorModule.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "DataSchemaMapping/GorgeousDataSchemaMappingAssetMenu.h"
#include "DataSchemaMapping/GorgeousDataSchemaMappingEditorToolkit.h"
#include "DataSchemaMapping/GorgeousDataSchemaMappingDetailCustomization.h"
#include "DataSchemaMapping/GorgeousDataSchemaMapping_DA.h"
#include "GorgeousCoreMinimalShared.h"
#include "GorgeousCoreEditorUtilitiesMinimalShared.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "IMessageLogListing.h"
#include "MessageLogModule.h"
#include "PropertyEditorModule.h"
#include "ToolMenus.h"
#include "LibraryWizard/SGorgeousLibraryView.h"
#include "Widgets/Docking/SDockTab.h"
#include "DataRegistry.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManagerSettings.h"
#include "EditorValidatorSubsystem.h"
#include "Containers/Ticker.h"
#include "UnrealEdMisc.h"
#include "GeneralSystems/GeneralSystemConfiguration_PDA.h"
#include "Interfaces/IPluginManager.h"
#include "Validators/GorgeousGeneralSystemValidator.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIMessageConfig_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIOverlayConfig_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousInputBinding_DA.h"
#include "DetailExtensions/GorgeousDetailExtension.h"
#include "DetailExtensions/GorgeousTestDetailExtension.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousPrimaryGameLayout.h"
#include "GorgeousCoreBlueprintTypes.h"
#include "DetailCustomizations/GorgeousGlobalDetailCustomization.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIState_DA.h"
#include "Validators/GorgeousCommonUIFoundationSystemValidator.h"
//<-------------------------------------------------------------------------->

#if 0
#include "GorgeousRainbowPinFactory.h"
#include "BlueprintActionDatabase.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphUtilities.h"
#include "K2Nodes/GorgeousK2Node_Get.h"
#include "K2Nodes/GorgeousK2Node_Set.h"
#endif

namespace
{
	FToolMenuOwnerScoped GDebugMenuOwner{TEXT("GorgeousCoreEditorUtilities")};
	static const FName GorgeousLibraryTabName("GorgeousLibrary");

	void RegisterLibraryMenuEntry()
	{
		if (!UToolMenus::IsToolMenuUIEnabled())
		{
			return;
		}

		FTabSpawnerEntry& Spawner = FGlobalTabmanager::Get()->RegisterNomadTabSpawner(GorgeousLibraryTabName, FOnSpawnTab::CreateLambda([](const FSpawnTabArgs& Args)
		{
			return SNew(SDockTab)
				.TabRole(ETabRole::NomadTab)
				[
					SNew(SGorgeousLibraryView)
				];
		}))
		.SetDisplayName(NSLOCTEXT("GorgeousCore", "LibraryTabTitle", "Gorgeous Library"))
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Package"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

		UToolMenus* ToolMenus = UToolMenus::Get();
		UToolMenu* WindowMenu = ToolMenus->ExtendMenu("LevelEditor.MainMenu.Window");
		if (!WindowMenu)
		{
			return;
		}

		FToolMenuSection& Section = WindowMenu->FindOrAddSection("GetContent");
		
		Section.AddMenuEntry(
			"OpenGorgeousLibrary",
			NSLOCTEXT("GorgeousCore", "OpenLibrary", "Gorgeous Library"),
			NSLOCTEXT("GorgeousCore", "OpenLibraryTooltip", "Open the Gorgeous Library to manage templates and assets."),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Package"),
			FUIAction(FExecuteAction::CreateLambda([]()
			{
				FGlobalTabmanager::Get()->TryInvokeTab(GorgeousLibraryTabName);
			}))
		);
	}
	inline bool GBDataRegistryValidationTriggered = false;
	inline bool GBValidationRan = false;

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

		const UToolMenus* ToolMenus = UToolMenus::Get();
		if (!ToolMenus)
		{
			return;
		}

		ToolMenus->UnregisterOwner(GDebugMenuOwner.GetOwner());
	}
	
	void RunGorgeousCoreStartupValidation(IAssetRegistry& AssetRegistry)
	{
		if (GBValidationRan)
		{
			GT_I_LOG("GorgeousCoreEditor",
				TEXT("Startup validation already ran — skipping duplicate."));
			return;
		}
		GBValidationRan = true;
		
		// Ensure system paths are registered in the Asset Manager
		if (UGorgeousGeneralSystemValidator* Validator = GetMutableDefault<UGorgeousGeneralSystemValidator>())
		{
			Validator->DiscoverAndRegisterGorgeousPluginSystems();
		}

		// Trigger the centralized system validation pass
		UGorgeousGeneralSystemValidator::RequestSystemValidationScan();
		
		GT_I_LOG("GorgeousCoreEditor",
			TEXT("Gorgeous validators are now active and will validate assets on load/save."));
	}
}

#if WITH_EDITOR
void FGorgeousCoreEditorModule::ValidateGorgeousModule(FDataValidationContext& InContext)
{
		// 1. Viewport Client Check
		UGorgeousCommonUIFoundationSystemValidator::ValidateViewportClient();

		// 2. Data Registry Discovery and Validation
		TArray<FString> ScannedDirs = UGorgeousGeneralSystemValidator::GetGorgeousSystemDirectories();
		if (ScannedDirs.Num() > 0)
		{
			FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
			IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

			for (const FString& Dir : ScannedDirs)
			{
				FARFilter Filter;
				Filter.ClassPaths.Add(UDataRegistry::StaticClass()->GetClassPathName());
				Filter.PackagePaths.Add(FName(*Dir));
				Filter.bRecursivePaths = true;

				TArray<FAssetData> FoundAssets;
				AssetRegistry.GetAssets(Filter, FoundAssets);

				if (FoundAssets.Num() > 0)
				{
					UGorgeousGeneralSystemValidator::QueueAssetsForAsyncValidation(FoundAssets);
					
					GT_I_LOG("GorgeousCoreEditor",
						TEXT("DataRegistry validation queued for %s. %d assets added."),
						*Dir, FoundAssets.Num());
				}
			}
		}
	}
#endif

//=============================================================================
// FGorgeousCoreEditorModule Implementation
//=============================================================================

void FGorgeousCoreEditorModule::GorgeousStartupModule()
{
	//@TODO: Planned for version 1.1 and upwards
	//Factory = MakeShareable(new FRainbowPinFactory());
	//FEdGraphUtilities::RegisterVisualNodeFactory(Factory);

	//RainbowPinFactory = MakeShareable(new FGorgeousRainbowGraphPinFactory());
	//FEdGraphUtilities::RegisterVisualPinFactory(RainbowPinFactory);

	//RainbowConnectionFactory = MakeShareable(new FGorgeousRainbowConnectionFactory());
	//FEdGraphUtilities::RegisterVisualPinConnectionFactory(RainbowConnectionFactory);
	
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomClassLayout(
		UGorgeousDataSchemaMapping_DA::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FGorgeousDataSchemaMappingDetailCustomization::MakeInstance));

	// Register Global Detail Customization for all Objects
	PropertyEditorModule.RegisterCustomClassLayout(
		UObject::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FGorgeousGlobalDetailCustomization::MakeInstance));

	// Register Default Extensions
	FGorgeousGlobalDetailCustomization::RegisterExtension(NewObject<UGorgeousTestDetailExtension>());

	PropertyEditorModule.NotifyCustomizationModuleChanged();

	GORGEOUS_REGISTER_STYLE_SET(ModuleStyleSet, "GorgeousCoreEditorStyle", "GorgeousCore", {
		GORGEOUS_STYLE_SET_BRUSHES(TEXT("ObjectVariable"), TEXT("variable-cube"), TEXT("GorgeousObjectVariableBlueprint"));
		GORGEOUS_STYLE_SET_BRUSHES(TEXT("ConditionalObjectChooser"), TEXT("chooser-funnel"), TEXT("GorgeousConditionalObjectChooserBlueprint"));
		GORGEOUS_STYLE_SET_BRUSHES(TEXT("Condition"), TEXT("condition"), TEXT("GorgeousConditionBlueprint"));

		GORGEOUS_STYLE_SET_NATIVE_CLASS_BRUSHES(TEXT("variable-cube"), TEXT("GorgeousObjectVariable"));
		GORGEOUS_STYLE_SET_NATIVE_CLASS_BRUSHES(TEXT("chooser-funnel"), TEXT("GorgeousConditionalObjectChooser"));
		GORGEOUS_STYLE_SET_NATIVE_CLASS_BRUSHES(TEXT("condition"), TEXT("GorgeousCondition"));

		GORGEOUS_STYLE_MAP_PARENT_CLASS_BRUSHES(TEXT("GorgeousGameInstanceBlueprint"), TEXT("Object"));
		GORGEOUS_STYLE_MAP_PARENT_CLASS_BRUSHES(TEXT("GorgeousGameModeBlueprint"), TEXT("GameModeBase"));
		GORGEOUS_STYLE_MAP_PARENT_CLASS_BRUSHES(TEXT("GorgeousGameStateBlueprint"), TEXT("GameStateBase"));
		GORGEOUS_STYLE_MAP_PARENT_CLASS_BRUSHES(TEXT("GorgeousPlayerControllerBlueprint"), TEXT("PlayerController"));
		
		GORGEOUS_STYLE_MAP_ENGINE_SVG_BRUSHES(TEXT("GorgeousPlayerStateBlueprint"), TEXT("AssetIcons/Actor_64.svg"));
		GORGEOUS_STYLE_MAP_ENGINE_SVG_BRUSHES(TEXT("GorgeousWorldSettingsBlueprint"), TEXT("Common/WorldSettings.svg"));

		// Emoji font — bundled with the plugin so it works on all platforms
		GORGEOUS_STYLE_REGISTER_FONT(TEXT("GorgeousCore.EmojiFont"), TEXT("Fonts/NotoColorEmoji/NotoColorEmoji-Regular"), 14);
	});
	
	GORGEOUS_REGISTER_ASSET_CATEGORY("GorgeousThings", "Gorgeous Things");

	const FText CoreMenu = NSLOCTEXT("GorgeousCore", "Menu_GorgeousCore", "Gorgeous Core");
	const FText Menu_ConditionalChoosers = NSLOCTEXT("GorgeousCore", "Menu_ConditionalChoosers", "Conditional Object Choosers");
	const FText Menu_Conditions = NSLOCTEXT("GorgeousCore", "Menu_Conditions", "Conditions");
	const FText Menu_ObjectVariables = NSLOCTEXT("GorgeousCore", "Menu_ObjectVariables", "Object Variables");
	const FText Menu_QualityOfLife = NSLOCTEXT("GorgeousCore", "Menu_QualityOfLife", "Quality of Life");
	const FText Menu_GeneralSystems = NSLOCTEXT("GorgeousCore", "Menu_GeneralSystems", "General Systems");
	const FText Menu_CommonUI = NSLOCTEXT("GorgeousCore", "Menu_CommonUI", "Common UI");

	REGISTER_GORGEOUS_ASSET(GORGEOUS_MAKE_INFO_WITH_BRUSHES_AND_EDITOR(
		NSLOCTEXT("GorgeousCore", "DataSchemaMapping", "Gorgeous Data Schema Mapping"),
		UGorgeousDataSchemaMapping_DA::StaticClass(),
		FColor(214, 134, 52),
		FAppStyle::Get().GetBrush(TEXT("ClassIcon.DataTable")),
		FAppStyle::Get().GetBrush(TEXT("ClassThumbnail.DataTable")),
		[](const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
		{
			const EToolkitMode::Type ToolkitMode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
			for (UObject* Object : InObjects)
			{
				if (UGorgeousDataSchemaMapping_DA* SchemaMap = Cast<UGorgeousDataSchemaMapping_DA>(Object))
				{
					const TSharedRef<FGorgeousDataSchemaMappingEditorToolkit> NewEditor = MakeShared<FGorgeousDataSchemaMappingEditorToolkit>();
					NewEditor->Initialize(ToolkitMode, EditWithinLevelEditor, SchemaMap);
				}
			}
		},
		CoreMenu));
	
	REGISTER_GORGEOUS_ASSET(GORGEOUS_MAKE_INFO(
		NSLOCTEXT("GorgeousCore", "ObjectVariable", "Gorgeous Object Variable"),
		UGorgeousObjectVariableBlueprint::StaticClass(),
		FColor::Blue,
		TEXT("ObjectVariable"),
		CoreMenu));

	REGISTER_GORGEOUS_ASSET(GORGEOUS_MAKE_INFO(
		NSLOCTEXT("GorgeousCore", "ConditionalObjectChooser", "Gorgeous Conditional Object Chooser"),
		UGorgeousConditionalObjectChooserBlueprint::StaticClass(),
		FColor::Cyan,
		TEXT("ConditionalObjectChooser"),
		CoreMenu, Menu_ConditionalChoosers));

	REGISTER_GORGEOUS_ASSET(GORGEOUS_MAKE_INFO(
		NSLOCTEXT("GorgeousCore", "Condition", "Gorgeous Condition"),
		UGorgeousConditionBlueprint::StaticClass(),
		FColor::Turquoise,
		TEXT("Condition"),
		CoreMenu, Menu_ConditionalChoosers, Menu_Conditions));

	REGISTER_GORGEOUS_ASSET(GORGEOUS_MAKE_INFO(
		NSLOCTEXT("GorgeousCore", "GameInstance", "Gorgeous Game Instance"),
		UGorgeousGameInstanceBlueprint::StaticClass(),
		FColor::Blue,
		NAME_None,
		CoreMenu, Menu_QualityOfLife));

	REGISTER_GORGEOUS_ASSET(GORGEOUS_MAKE_INFO_WITH_BRUSHES(
		NSLOCTEXT("GorgeousCore", "GameMode", "Gorgeous Game Mode"),
		UGorgeousGameModeBlueprint::StaticClass(),
		FColor::Blue,
		FAppStyle::Get().GetBrush(TEXT("ClassIcon.GameModeBase")),
		FAppStyle::Get().GetBrush(TEXT("ClassThumbnail.GameModeBase")), 
		CoreMenu, Menu_QualityOfLife));

	REGISTER_GORGEOUS_ASSET(GORGEOUS_MAKE_INFO_WITH_BRUSHES(
		NSLOCTEXT("GorgeousCore", "GameState", "Gorgeous Game State"),
		UGorgeousGameStateBlueprint::StaticClass(),
		FColor::Blue,
		FAppStyle::Get().GetBrush(TEXT("ClassIcon.GameStateBase")),
		FAppStyle::Get().GetBrush(TEXT("ClassThumbnail.GameStateBase")),
		CoreMenu, Menu_QualityOfLife));

	REGISTER_GORGEOUS_ASSET(GORGEOUS_MAKE_INFO_WITH_BRUSHES(
		NSLOCTEXT("GorgeousCore", "PlayerController", "Gorgeous Player Controller"),
		UGorgeousPlayerControllerBlueprint::StaticClass(),
		FColor::Blue,
		
		FAppStyle::Get().GetBrush(TEXT("ClassIcon.PlayerController")),
		FAppStyle::Get().GetBrush(TEXT("ClassThumbnail.PlayerController")),
		CoreMenu, Menu_QualityOfLife));

	REGISTER_GORGEOUS_ASSET(GORGEOUS_MAKE_INFO_WITH_BRUSHES(
		NSLOCTEXT("GorgeousCore", "PlayerState", "Gorgeous Player State"),
		UGorgeousPlayerStateBlueprint::StaticClass(),
		FColor::Blue,
		
		ModuleStyleSet->GetBrush(TEXT("ClassIcon.GorgeousPlayerStateBlueprint")),
		ModuleStyleSet->GetBrush(TEXT("ClassThumbnail.GorgeousPlayerStateBlueprint")),
		CoreMenu, Menu_QualityOfLife));

	REGISTER_GORGEOUS_ASSET(GORGEOUS_MAKE_INFO_WITH_BRUSHES(
		NSLOCTEXT("GorgeousCore", "WorldSettings", "Gorgeous World Settings"),
		UGorgeousWorldSettingsBlueprint::StaticClass(),
		FColor::Blue,
		ModuleStyleSet->GetBrush(TEXT("ClassIcon.GorgeousWorldSettingsBlueprint")),
		ModuleStyleSet->GetBrush(TEXT("ClassThumbnail.GorgeousWorldSettingsBlueprint")),
		CoreMenu, Menu_QualityOfLife));
	
	REGISTER_GORGEOUS_ASSET(GORGEOUS_MAKE_INFO_WITH_BRUSHES(
		NSLOCTEXT("GorgeousCore", "HUD", "Gorgeous HUD"),
		UGorgeousHUDBlueprint::StaticClass(),
		FColor::Blue,
		ModuleStyleSet->GetBrush(TEXT("ClassIcon.HUD")),
		ModuleStyleSet->GetBrush(TEXT("ClassThumbnail.HUD")),
		CoreMenu, Menu_QualityOfLife));

	// --- Common UI Foundation assets ---
	REGISTER_GORGEOUS_ASSET(GORGEOUS_MAKE_INFO_WITH_BRUSHES(
		NSLOCTEXT("GorgeousCore", "UITheme", "Gorgeous UI Theme"),
		UGorgeousUITheme_DA::StaticClass(),
		FColor(140,40,200),
		FAppStyle::Get().GetBrush(TEXT("ClassIcon.DataAsset")),
		FAppStyle::Get().GetBrush(TEXT("ClassThumbnail.DataAsset")),
		CoreMenu, Menu_GeneralSystems, Menu_CommonUI));

	REGISTER_GORGEOUS_ASSET(GORGEOUS_MAKE_INFO_WITH_BRUSHES(
		NSLOCTEXT("GorgeousCore", "UIMessageConfig", "Gorgeous UI Message Config"),
		UGorgeousUIMessageConfig_DA::StaticClass(),
		FColor(255,140,40),
		FAppStyle::Get().GetBrush(TEXT("ClassIcon.DataAsset")),
		FAppStyle::Get().GetBrush(TEXT("ClassThumbnail.DataAsset")),
		CoreMenu, Menu_GeneralSystems, Menu_CommonUI));

	REGISTER_GORGEOUS_ASSET(GORGEOUS_MAKE_INFO_WITH_BRUSHES(
		NSLOCTEXT("GorgeousCore", "CommonUIState", "Gorgeous Common UI State"),
		UGorgeousUIState_DA::StaticClass(),
		FColor(60,140,220),
		FAppStyle::Get().GetBrush(TEXT("ClassIcon.DataAsset")),
		FAppStyle::Get().GetBrush(TEXT("ClassThumbnail.DataAsset")),
		CoreMenu, Menu_GeneralSystems, Menu_CommonUI));

	REGISTER_GORGEOUS_ASSET(GORGEOUS_MAKE_INFO_WITH_BRUSHES(
		NSLOCTEXT("GorgeousCore", "CommonUIOverlayConfig", "Gorgeous Common UI Overlay Config"),
		UGorgeousUIOverlayConfig_DA::StaticClass(),
		FColor(90,200,140),
		FAppStyle::Get().GetBrush(TEXT("ClassIcon.DataAsset")),
		FAppStyle::Get().GetBrush(TEXT("ClassThumbnail.DataAsset")),
		CoreMenu, Menu_GeneralSystems, Menu_CommonUI));

	REGISTER_GORGEOUS_ASSET(GORGEOUS_MAKE_INFO_WITH_BRUSHES(
		NSLOCTEXT("GorgeousCore", "InputBinding", "Gorgeous Input Binding"),
		UGorgeousInputBinding_DA::StaticClass(),
		FColor(200, 40, 90),
		FAppStyle::Get().GetBrush(TEXT("ClassIcon.DataAsset")),
		FAppStyle::Get().GetBrush(TEXT("ClassThumbnail.DataAsset")),
		CoreMenu, Menu_GeneralSystems, Menu_CommonUI));

	BeginPIEHandle = FEditorDelegates::BeginPIE.AddLambda([](bool bIsSimulating)
	{
		if (FModuleManager::Get().IsModuleLoaded("MessageLog") || FModuleManager::Get().LoadModule("MessageLog"))
		{
			FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
			const FName ListingName = GorgeousLogging::GetLoggingSettingsSnapshot().MessageLogListingName;
			MessageLogModule.GetLogListing(ListingName)->ClearMessages();
		}

		if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
		{
			if (Subsystem->IsDebugPanelOpen())
			{
				Subsystem->ShowInGamePanel();
			}
			
			if (Subsystem->GetQueuedTestCount() > 0)
			{
				Subsystem->RunQueuedTests();
				Subsystem->RefreshDebugPanel();
			}
		}
	});
	
	EndPIEHandle = FEditorDelegates::EndPIE.AddLambda([](bool bIsSimulating)
	{
		if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
		{
			Subsystem->HideInGamePanel();
			Subsystem->RefreshDebugPanel();
		}
	});

	FGorgeousDataSchemaMappingAssetMenu::Register();
	
	RegisterLibraryMenuEntry();
	RegisterDebugMenuEntry();
	
	FCoreDelegates::OnPostEngineInit.AddLambda([this]()
	{
		if (!GEditor)
			return;

		// Delay execution to ensure everything is fully initialized
		FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateLambda([this](float)
			{
				FAssetRegistryModule& ARM =
					FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

				RunGorgeousCoreStartupValidation(ARM.Get());

				return false; // run once
			}),
			2.0f
		);
	});
}

void FGorgeousCoreEditorModule::GorgeousShutdownModule()
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
	
	//@TODO: Planned for version 1.1 and upwards
	//FEdGraphUtilities::UnregisterVisualNodeFactory(Factory);
	//FEdGraphUtilities::UnregisterVisualPinFactory(RainbowPinFactory);
	//FEdGraphUtilities::UnregisterVisualPinConnectionFactory(RainbowConnectionFactory);

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.UnregisterCustomClassLayout(UGorgeousDataSchemaMapping_DA::StaticClass()->GetFName());
		PropertyEditorModule.UnregisterCustomClassLayout(UObject::StaticClass()->GetFName());
		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}

	FGorgeousDataSchemaMappingAssetMenu::Unregister();
	
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(GorgeousLibraryTabName);

	UnregisterDebugMenuEntry();
	
	UNREGISTER_GORGEOUS_ASSETS;
	GORGEOUS_UNREGISTER_STYLE_SET(ModuleStyleSet);
}

IMPLEMENT_MODULE(FGorgeousCoreEditorModule, GorgeousCoreEditor)