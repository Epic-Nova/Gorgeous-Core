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
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "DataSchemaMapping/GorgeousDataSchemaMappingAssetMenu.h"
#include "DataSchemaMapping/GorgeousDataSchemaMappingEditorToolkit.h"
#include "DataSchemaMapping/GorgeousDataSchemaMappingDetailCustomization.h"
#include "DataSchemaMapping/GorgeousDataSchemaMapping_DA.h"
#include "GorgeousCoreMinimalShared.h"
#include "GorgeousCoreEditorUtilitiesMinimalShared.h"
#include "NudgeHelper/GorgeousNudgeSubsystem.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "IMessageLogListing.h"
#include "MessageLogModule.h"
#include "PropertyEditorModule.h"
#include "PropertyTypeCustomizations/GorgeousFunctionalStructurePropertyTypeCustomization.h"
#include "PropertyTypeCustomizations/GorgeousObjectVariablePropertyTypeCustomization.h"
#include "ObjectVariables/GorgeousObjectVariableTrunk.h"
#include "FunctionalStructures/GorgeousFunctionalStructure.h"
#include "FeedbackEffects/GorgeousFeedbackEffect.h"
#include "DetailCustomizations/GorgeousFeedbackEffectCustomization.h"
#include "DetailCustomizations/GorgeousProtectionSettingsCustomization.h"
#include "UObject/UObjectIterator.h"
#include "ToolMenus.h"
#include "ISettingsModule.h"
#include "LibraryWizard/GorgeousLibrarySettings.h"
#include "LibraryWizard/GorgeousCoreLibraryParticipant.h"
#include "LibraryWizard/GorgeousRecompilationHelper.h"
#include "LibraryWizard/SGorgeousLibraryView.h"
#include "Widgets/Docking/SDockTab.h"
#include "DataRegistry.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManagerSettings.h"
#include "Engine/Blueprint.h"
#include "EditorValidatorSubsystem.h"
#include "HAL/PlatformProcess.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Toolkits/AssetEditorToolkitMenuContext.h"
#include "Containers/Ticker.h"
#include "Framework/Application/SlateApplication.h"
#include "UnrealEdMisc.h"
#include "GeneralSystems/GeneralSystemConfiguration_PDA.h"
#include "Interfaces/IPluginManager.h"
#include "Validators/GorgeousGeneralSystemValidator.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIMessageConfig_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUIOverlayConfig_DA.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousInputBinding_DA.h"
#include "DetailExtensions/GorgeousDetailExtension.h"
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

//=============================================================================
// Helpers
//=============================================================================

namespace
{
	static const FName GGorgeousHelpMenuOwnerName{TEXT("GorgeousCoreEditor.HelpMenu")};
	static const FName GorgeousLibraryTabName("GorgeousLibrary");
	static FDelegateHandle GGorgeousHelpMenuStartupCallbackHandle;
	static FDelegateHandle GPostEngineInitHandle;
	static FTSTicker::FDelegateHandle GStartupValidationTickerHandle;
	static FTSTicker::FDelegateHandle GRecompilationCheckTickerHandle;
	static FTSTicker::FDelegateHandle GGeneralSystemActivationTickerHandle;
	static bool GBGorgeousHelpMenusRegistered = false;

void RunGorgeousCoreStartupValidation(IAssetRegistry& AssetRegistry);

	struct FGorgeousDocumentationLink
	{
		FName EntryName;
		FText Label;
		FString Url;
	};

	UClass* GetAssetDocumentationClass(const UObject* Asset)
	{
		if (const UBlueprint* Blueprint = Cast<UBlueprint>(Asset))
		{
			if (Blueprint->GeneratedClass)
			{
				return Blueprint->GeneratedClass;
			}
			if (Blueprint->SkeletonGeneratedClass)
			{
				return Blueprint->SkeletonGeneratedClass;
			}

			return Blueprint->ParentClass.Get();
		}

		return Asset ? Asset->GetClass() : nullptr;
	}

	UClass* GetEditedAssetDocumentationClass(const FAssetEditorToolkit& AssetEditorToolkit)
	{
		const TArray<UObject*>* EditedAssets = AssetEditorToolkit.GetObjectsCurrentlyBeingEdited();
		return EditedAssets && EditedAssets->Num() == 1 ? GetAssetDocumentationClass((*EditedAssets)[0]) : nullptr;
	}

	FString GetDocumentationMetadata(const UClass* FocusedClass, const TCHAR* MetadataKey)
	{
		for (const UClass* Class = FocusedClass; Class; Class = Class->GetSuperClass())
		{
			const FString Url = Class->GetMetaData(MetadataKey);
			if (!Url.IsEmpty())
			{
				return Url;
			}
		}
		return FString();
	}

	void ShowGorgeousNudgeCarousel()
	{
		if (GEditor)
		{
			if (UGorgeousNudgeSubsystem* Nudges = GEditor->GetEditorSubsystem<UGorgeousNudgeSubsystem>())
			{
				Nudges->ShowNudgeCarousel();
			}
		}
	}

	void PopulateGorgeousBlueprintDocumentationMenu(UToolMenu* Menu)
	{
		const UAssetEditorToolkitMenuContext* Context = Menu->FindContext<UAssetEditorToolkitMenuContext>();
		const TSharedPtr<FAssetEditorToolkit> AssetEditorToolkit = Context ? Context->Toolkit.Pin() : nullptr;
		UClass* FocusedClass = AssetEditorToolkit ? GetEditedAssetDocumentationClass(*AssetEditorToolkit) : nullptr;
		if (!FocusedClass)
		{
			return;
		}

		TArray<FGorgeousDocumentationLink> DocumentationLinks;
		const auto AddDocumentationLink = [&DocumentationLinks, FocusedClass](const TCHAR* MetadataKey, const FName EntryName, const FText& Label)
		{
			const FString Url = GetDocumentationMetadata(FocusedClass, MetadataKey);
			if (!Url.IsEmpty())
			{
				DocumentationLinks.Add({EntryName, Label, Url});
			}
		};

		AddDocumentationLink(TEXT("DocumentationOverview"), TEXT("GorgeousDocumentationOverview"), NSLOCTEXT("GorgeousCore", "ContextDocumentationOverview", "Gorgeous: Overview"));
		AddDocumentationLink(TEXT("DocumentationAPI"), TEXT("GorgeousDocumentationAPI"), NSLOCTEXT("GorgeousCore", "ContextDocumentationAPI", "Gorgeous: API Reference"));
		AddDocumentationLink(TEXT("DocumentationExamples"), TEXT("GorgeousDocumentationExamples"), NSLOCTEXT("GorgeousCore", "ContextDocumentationExamples", "Gorgeous: Examples"));
		if (DocumentationLinks.IsEmpty())
		{
			return;
		}

		FToolMenuSection& Section = Menu->AddSection(
			"GorgeousContextDocumentation",
			NSLOCTEXT("GorgeousCore", "GorgeousContextDocumentationSection", "Gorgeous Documentation"),
			FToolMenuInsert("HelpResources", EToolMenuInsertType::After));
		for (const FGorgeousDocumentationLink& DocumentationLink : DocumentationLinks)
		{
			Section.AddMenuEntry(
				DocumentationLink.EntryName,
				DocumentationLink.Label,
				FText::Format(NSLOCTEXT("GorgeousCore", "ContextDocumentationTooltip", "Open Gorgeous documentation: {0}"), DocumentationLink.Label),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Help"),
				FUIAction(FExecuteAction::CreateLambda([Url = DocumentationLink.Url]()
				{
					FPlatformProcess::LaunchURL(*Url, nullptr, nullptr);
				})));
		}
	}

	void RegisterGorgeousHelpMenus()
	{
		if (GBGorgeousHelpMenusRegistered)
		{
			return;
		}

		UToolMenus* ToolMenus = UToolMenus::TryGet();
		if (!ToolMenus)
		{
			return;
		}

		FToolMenuOwnerScoped OwnerScoped(GGorgeousHelpMenuOwnerName);
		UToolMenu* LevelEditorHelpMenu = ToolMenus->ExtendMenu("LevelEditor.MainMenu.Help");
		FToolMenuSection& LevelEditorSection = LevelEditorHelpMenu->FindOrAddSection("GorgeousThings");
		LevelEditorSection.Label = NSLOCTEXT("GorgeousCore", "GorgeousHelpSection", "Gorgeous Things");
		LevelEditorSection.InsertPosition = FToolMenuInsert("HelpResources", EToolMenuInsertType::After);
		LevelEditorSection.AddMenuEntry(
			"ShowGorgeousNews",
			NSLOCTEXT("GorgeousCore", "ShowGorgeousNews", "Gorgeous Updates & News"),
			NSLOCTEXT("GorgeousCore", "ShowGorgeousNewsTooltip", "Show relevant Gorgeous plugin updates, system updates, tutorials, and ecosystem news."),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Info"),
			FUIAction(FExecuteAction::CreateStatic(&ShowGorgeousNudgeCarousel)));

		UToolMenu* BlueprintEditorHelpMenu = ToolMenus->ExtendMenu("AssetEditor.BlueprintEditor.MainMenu.Help");
		BlueprintEditorHelpMenu->AddDynamicSection(
			"GorgeousContextDocumentationDynamic",
			FNewToolMenuDelegate::CreateStatic(&PopulateGorgeousBlueprintDocumentationMenu),
			FToolMenuInsert("HelpResources", EToolMenuInsertType::After));

		GBGorgeousHelpMenusRegistered = true;
	}

	void UnregisterGorgeousHelpMenus()
	{
		if (GGorgeousHelpMenuStartupCallbackHandle.IsValid())
		{
			UToolMenus::UnRegisterStartupCallback(GGorgeousHelpMenuStartupCallbackHandle);
			GGorgeousHelpMenuStartupCallbackHandle.Reset();
		}

		if (UToolMenus* ToolMenus = UToolMenus::TryGet())
		{
			ToolMenus->UnregisterOwner(GGorgeousHelpMenuOwnerName);
		}

		GBGorgeousHelpMenusRegistered = false;
	}

	void HandlePostEngineInit()
	{
		if (!GEditor)
		{
			return;
		}

		GRecompilationCheckTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateLambda([](float)
			{
				if (!FSlateApplication::IsInitialized() || FSlateApplication::Get().GetTopLevelWindows().Num() == 0)
				{
					return true;
				}

				GRecompilationCheckTickerHandle.Reset();
				UGorgeousRecompilationHelper::CheckRecompilationRequirement();
				return false;
			}),
			0.0f);

		GGeneralSystemActivationTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateLambda([](float)
			{
				if (!FSlateApplication::IsInitialized() || FSlateApplication::Get().GetTopLevelWindows().Num() == 0)
				{
					return true;
				}

				GGeneralSystemActivationTickerHandle.Reset();
				UGeneralSystemConfiguration_PDA::ActivateAllSystemConfigurations();
				return false;
			}),
			0.0f);

		GStartupValidationTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateLambda([](float)
			{
				GStartupValidationTickerHandle.Reset();
				FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
				RunGorgeousCoreStartupValidation(AssetRegistryModule.Get());
				return false;
			}),
			2.0f);
	}

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

	void RunGorgeousCoreStartupValidation(IAssetRegistry& AssetRegistry)
	{
		if (GBValidationRan)
		{
			GT_I_LOG("GorgeousCoreEditor",
				TEXT("Startup validation already ran, skipping duplicate."));
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

static TArray<FName> GFunctionalStructureCustomizationNames;

void FGorgeousCoreEditorModule::GorgeousStartupModule()
{
	//@TODO: Planned for version 1.1 and upwards
	//Factory = MakeShareable(new FRainbowPinFactory());
	//FEdGraphUtilities::RegisterVisualNodeFactory(Factory);

	//RainbowPinFactory = MakeShareable(new FGorgeousRainbowGraphPinFactory());
	//FEdGraphUtilities::RegisterVisualPinFactory(RainbowPinFactory);

	//RainbowConnectionFactory = MakeShareable(new FGorgeousRainbowConnectionFactory());
	//FEdGraphUtilities::RegisterVisualPinConnectionFactory(RainbowConnectionFactory);
	


	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(&RegisterLibraryMenuEntry));

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(FGorgeousObjectVariableSerializedPayload::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGorgeousObjectVariablePropertyTypeCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomClassLayout(
		UGorgeousDataSchemaMapping_DA::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FGorgeousDataSchemaMappingDetailCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomClassLayout(
		UGorgeousFeedbackEffect::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FGorgeousFeedbackEffectCustomization::MakeInstance));
	if (!IsRunningCommandlet())
	{
		PropertyEditorModule.RegisterCustomClassLayout(
			UGorgeousUsefulSettings::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FGorgeousProtectionSettingsCustomization::MakeInstance));
	}

	for (TObjectIterator<UScriptStruct> It; It; ++It)
	{
		if (It->IsChildOf(FGorgeousFunctionalStructure_S::StaticStruct()))
		{
			PropertyEditorModule.RegisterCustomPropertyTypeLayout(It->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGorgeousFunctionalStructurePropertyTypeCustomization::MakeInstance));
			GFunctionalStructureCustomizationNames.AddUnique(It->GetFName());
		}
	}

	// Metadata identifiers apply only to explicitly tagged properties at any nesting depth.
	FGorgeousGlobalDetailCustomization::RegisterPropertyCustomizations();

	// Register Default Extensions
	// Example detail extensions are registered explicitly by development tools, not by customer editor startup.

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

		// Emoji font, bundled with the plugin so it works on all platforms
		GorgeousStyleRegistration::GGorgeousStyleSet->Set(TEXT("GorgeousCore.EmojiFont"), FAppStyle::Get().GetFontStyle(TEXT("NormalFont")));
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
		ModuleStyleSet->GetBrush(TEXT("ClassIcon.Blueprint")),
		ModuleStyleSet->GetBrush(TEXT("ClassThumbnail.Blueprint")),
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

	if (ISettingsModule* Settings = FModuleManager::LoadModulePtr<ISettingsModule>(TEXT("Settings")))
	{
		Settings->RegisterSettings(TEXT("Project"), TEXT("Gorgeous Things"), TEXT("CoreLibrary"),
			NSLOCTEXT("GorgeousLibrary", "SettingsTitle", "Core Library and Privacy"),
			NSLOCTEXT("GorgeousLibrary", "SettingsDescription", "Gorgeous Library, update consent and validation settings."),
			GetMutableDefault<UGorgeousLibrarySettings>());
	}

	CoreLibraryParticipant = MakeUnique<FGorgeousCoreLibraryParticipant>();
	LibraryParticipant = CoreLibraryParticipant.Get();

	FGorgeousDataSchemaMappingAssetMenu::Register();
	
	RegisterLibraryMenuEntry();
	UToolMenus::Get()->RefreshAllWidgets();

	GGorgeousHelpMenuStartupCallbackHandle = UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateStatic(&RegisterGorgeousHelpMenus));
	GPostEngineInitHandle = GORGEOUS_58_SWITCH(FCoreDelegates::OnPostEngineInit, FCoreDelegates::GetOnPostEngineInit()).AddStatic(&HandlePostEngineInit);
}

void FGorgeousCoreEditorModule::GorgeousShutdownModule()
{
	FGorgeousGlobalDetailCustomization::UnregisterPropertyCustomizations();
	FGorgeousGlobalDetailCustomization::ClearExtensions();
	if (GPostEngineInitHandle.IsValid())
	{
		GORGEOUS_58_SWITCH(FCoreDelegates::OnPostEngineInit, FCoreDelegates::GetOnPostEngineInit()).Remove(GPostEngineInitHandle);
		GPostEngineInitHandle.Reset();
	}
	if (GStartupValidationTickerHandle.IsValid())
	{
		FTSTicker::RemoveTicker(GStartupValidationTickerHandle);
		GStartupValidationTickerHandle.Reset();
	}
	if (GRecompilationCheckTickerHandle.IsValid())
	{
		FTSTicker::RemoveTicker(GRecompilationCheckTickerHandle);
		GRecompilationCheckTickerHandle.Reset();
	}
	
	//@TODO: Planned for version 1.1 and upwards
	//FEdGraphUtilities::UnregisterVisualNodeFactory(Factory);
	//FEdGraphUtilities::UnregisterVisualPinFactory(RainbowPinFactory);
	//FEdGraphUtilities::UnregisterVisualPinConnectionFactory(RainbowConnectionFactory);

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Gorgeous", "Core");
		SettingsModule->UnregisterSettings(TEXT("Project"), TEXT("Gorgeous Things"), TEXT("CoreLibrary"));
	}

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.UnregisterCustomPropertyTypeLayout(FGorgeousObjectVariableSerializedPayload::StaticStruct()->GetFName());
		for (FName StructName : GFunctionalStructureCustomizationNames) PropertyEditorModule.UnregisterCustomPropertyTypeLayout(StructName);
		GFunctionalStructureCustomizationNames.Empty();
		PropertyEditorModule.UnregisterCustomClassLayout(UGorgeousDataSchemaMapping_DA::StaticClass()->GetFName());
		PropertyEditorModule.UnregisterCustomClassLayout(UGorgeousFeedbackEffect::StaticClass()->GetFName());
		if (!IsRunningCommandlet())
			PropertyEditorModule.UnregisterCustomClassLayout(UGorgeousUsefulSettings::StaticClass()->GetFName());
		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}

	FGorgeousDataSchemaMappingAssetMenu::Unregister();
	
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(GorgeousLibraryTabName);

	UnregisterGorgeousHelpMenus();
	
	UNREGISTER_GORGEOUS_ASSETS;
	GORGEOUS_UNREGISTER_STYLE_SET(ModuleStyleSet);
}

IMPLEMENT_MODULE(FGorgeousCoreEditorModule, GorgeousCoreEditor)
