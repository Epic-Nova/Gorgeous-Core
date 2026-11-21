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
#include "ModuleCore/GorgeousCoreSettingsPages.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "DetailCustomizations/GorgeousProtectionSettingsCustomization.h"
#include "ExtensionResourceGuard/GorgeousExtensionResourceGuard.h"
#include "GeneralSystems/GeneralSystemConfiguration_PDA.h"
#include "GeneralSystems/GorgeousCompiledSystemGuards.h"
#include "GeneralSystems/GorgeousSystemProjectSettings.h"
#include "Helpers/Macros/GorgeousVersionHelperMacros.h"
#include "ModuleCore/GorgeousCoreDeveloperSettings.h"
#include "LibraryWizard/GorgeousLibrarySettings.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "ContentBrowserMenuContexts.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/DataAsset.h"
#include "IDetailsView.h"
#include "IDetailCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "ISettingsModule.h"
#include "IStructureDetailsView.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectIterator.h"
#include "UObject/StructOnScope.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
//<-------------------------------------------------------------------------->

#define LOCTEXT_NAMESPACE "GorgeousCoreSettingsPages"

//=============================================================================
// Helpers
//=============================================================================

namespace
{
	const FName CoreProjectContainer(TEXT("Project"));
	const FName CoreSettingsCategory(TEXT("Gorgeous Things"));
	const FName CoreGeneralPage(TEXT("CoreLibrary"));
	const FName CoreDebuggingPage(TEXT("CoreDebugging"));
	const FName CoreAutoReplicationPage(TEXT("CoreAutoReplication"));
	const FName CoreSystemsPage(TEXT("CoreSystems"));
	const FName CoreSystemSettingsMenuOwner(TEXT("GorgeousCoreSystemSettings"));

	// ISettingsModule stores weak widget references.
	TMap<FName, TSharedRef<SWidget>> CoreSettingsWidgets;
	TWeakPtr<SScrollBox> CoreSystemsScrollBox;
	TMultiMap<FName, TWeakPtr<SExpandableArea>> CoreSystemSettingsGroups;
	TMultiMap<FName, TWeakPtr<SExpandableArea>> CoreAuthoredSettingsGroups;
	TMap<FName, TWeakPtr<SVerticalBox>> CoreNativeSystemSettingsBodies;
	TWeakPtr<SVerticalBox> CoreAuthoredSettingsBody;
	TArray<TSharedRef<IStructureDetailsView>> CoreAuthoredDetails;
	TArray<TStrongObjectPtr<UGeneralSystemConfiguration_PDA>> CoreAuthoredConfigurations;
	FDelegateHandle CoreSettingsFilesLoaded;
	FDelegateHandle CoreSettingsAssetAdded;
	FDelegateHandle CoreSettingsAssetRemoved;
	FDelegateHandle CoreSettingsAssetUpdated;
	FDelegateHandle CoreSettingsAssetRenamed;

	/** Applies local category defaults without replacing class-specific detail customizations. */
	class FCoreSettingsCategoryDefaults final : public IDetailCustomization
	{
	public:
		// Retains any generic customization already supplied for this view.
		explicit FCoreSettingsCategoryDefaults(const FOnGetDetailCustomizationInstance& Previous)
		{
			if (Previous.IsBound()) PreviousCustomization = Previous.Execute();
		}

		// Preserves the existing customization lifecycle when a layout is discarded.
		virtual void PendingDelete() override
		{
			if (PreviousCustomization) PreviousCustomization->PendingDelete();
		}

		// Keeps reference-based generic layouts and adds collapsed category defaults.
		virtual void CustomizeDetails(IDetailLayoutBuilder& Builder) override
		{
			if (PreviousCustomization) PreviousCustomization->CustomizeDetails(Builder);
			AddCategoryDefaults(Builder);
		}

		// Keeps shared-layout overrides used by customizers that retain a weak builder pointer.
		virtual void CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& Builder) override
		{
			if (PreviousCustomization) PreviousCustomization->CustomizeDetails(Builder);
			if (Builder) AddCategoryDefaults(*Builder);
		}

	private:
		/**
		 * Sets defaults after all categories have been generated without changing their sort order.
		 *
		 * @param Builder The local layout being initialized.
		 */
		static void AddCategoryDefaults(IDetailLayoutBuilder& Builder)
		{
			Builder.SortCategories([](const TMap<FName, IDetailCategoryBuilder*>& Categories)
			{
				for (const auto& Category : Categories)
				{
					if (Category.Value) Category.Value->InitiallyCollapsed(true);
				}
			});
		}

		// Holds the prior view-local customization through this layout's lifetime.
		TSharedPtr<IDetailCustomization> PreviousCustomization;
	};

	/**
	 * Scopes expansion state to the identifier belonging to this settings view.
	 *
	 * @param Args The arguments used to construct a Gorgeous settings view.
	 * @param ViewId The settings class or authored configuration identifying this view.
	 */
	void ConfigureCoreSettingsView(FDetailsViewArgs& Args, FName ViewId)
	{
		Args.ViewIdentifier = FName(*(FString(TEXT("GorgeousCore.ProjectSettings.")) + ViewId.ToString()));
#if GORGEOUS_58_SWITCH(0, 1)
		Args.ExpansionPersistenceMethod = FDetailsViewArgs::EExpansionPersistenceMethod::TransientPerViewIdentifier;
#endif
	}

	/**
	 * Adds local category defaults while retaining any pre-existing generic customization.
	 *
	 * @param Details The newly created Gorgeous settings view.
	 */
	void InitializeCoreSettingsView(IDetailsView& Details)
	{
		const FOnGetDetailCustomizationInstance Previous = Details.GetGenericLayoutDetailsDelegate();
		Details.SetGenericLayoutDetailsDelegate(FOnGetDetailCustomizationInstance::CreateLambda([Previous]
		{
			return MakeShared<FCoreSettingsCategoryDefaults>(Previous);
		}));
	}

	/**
	 * Collapses the initial tree once; subsequent refreshes retain the user's local choices.
	 *
	 * @param Details The view whose initial object or structure has just been assigned.
	 */
	void CollapseInitialCoreSettingsTree(IDetailsView& Details)
	{
#if GORGEOUS_58_SWITCH(0, 1)
		Details.SetRootExpansionStates(false, true);
#else
		// Older public APIs retain normal category defaults and remembered state.
		(void)Details;
#endif
	}

	TSharedRef<SExpandableArea> MakeCoreSettingsArea(const FText& Title, const TSharedRef<SWidget>& Body)
	{
		return SNew(SExpandableArea)
			.InitiallyCollapsed(false)
			.HeaderContent()[SNew(STextBlock).Text(Title)]
			.BodyContent()[Body];
	}

	void SaveCoreSettings(UObject& Settings)
	{
		const UClass* Class = Settings.GetClass();
		if (Class->HasAnyClassFlags(CLASS_DefaultConfig))
		{
			Settings.TryUpdateDefaultConfigFile();
		}
		else if (Class->HasAnyClassFlags(CLASS_GlobalUserConfig))
		{
			Settings.UpdateGlobalUserConfigFile();
		}
		else if (Class->HasAnyClassFlags(CLASS_ProjectUserConfig))
		{
			Settings.UpdateProjectUserConfigFile();
		}
		else
		{
			Settings.SaveConfig();
		}
	}

	TSharedRef<SExpandableArea> AddCoreSettingsGroup(SScrollBox& Page, FName ViewId, const FText& Title,
		UObject& Settings, FIsPropertyVisible Visibility = {}, FName System = NAME_None)
	{
		FDetailsViewArgs Args;
		ConfigureCoreSettingsView(Args, ViewId);
		Args.bUpdatesFromSelection = false;
		Args.bAllowSearch = false;
		Args.bShowOptions = false;
		Args.bHideSelectionTip = true;
		Args.bShowScrollBar = false;
		Args.NameAreaSettings = FDetailsViewArgs::HideNameArea;

		auto& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		const TSharedRef<IDetailsView> Details = PropertyEditor.CreateDetailView(Args);
		InitializeCoreSettingsView(*Details);
		if (ViewId == GorgeousCoreSettingsPages::EditorDiagnosticsViewId)
		{
			Details->RegisterInstancedCustomPropertyLayout(Settings.GetClass(),
				FOnGetDetailCustomizationInstance::CreateStatic(&FGorgeousProtectionSettingsCustomization::MakeInstance));
		}
		Details->SetIsPropertyVisibleDelegate(MoveTemp(Visibility));
		Details->SetObject(&Settings);
		CollapseInitialCoreSettingsTree(*Details);
		Details->OnFinishedChangingProperties().AddWeakLambda(&Settings,
			[&Settings](const FPropertyChangedEvent&) { SaveCoreSettings(Settings); });

		TSharedRef<SWidget> Body = Details;
		if (auto* Library = Cast<UGorgeousLibrarySettings>(&Settings))
		{
			Body = SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(6.f, 4.f)
				[
					SNew(STextBlock).AutoWrapText(true)
					.Text(LOCTEXT("OnlineServicesPurpose", "Choose which online services and notifications Gorgeous Things may use in this project."))
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(6.f, 2.f, 6.f, 8.f)
				[
					SNew(SButton)
					.Text(LOCTEXT("ConfigureOnlineServices", "Configure Online Services"))
					.OnClicked_Lambda([WeakSettings = TWeakObjectPtr<UGorgeousLibrarySettings>(Library)]
					{
						if (UGorgeousLibrarySettings* Current = WeakSettings.Get()) Current->ConfigureOnlineServices();
						return FReply::Handled();
					})
				]
				+ SVerticalBox::Slot().AutoHeight()[Details];
		}
		if (!System.IsNone())
		{
			const TSharedRef<SVerticalBox> Authored = SNew(SVerticalBox);
			Body = SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()[Details]
				+ SVerticalBox::Slot().AutoHeight()[Authored];
			CoreNativeSystemSettingsBodies.Add(System, Authored);
		}
		const TSharedRef<SExpandableArea> Area = MakeCoreSettingsArea(Title, Body);
		Page.AddSlot().Padding(6.f, 3.f)[Area];
		return Area;
	}

	FName GetCoreSettingsSystem(const UGeneralSystemConfiguration_PDA& Configuration)
	{
		FString Name = Configuration.GetPDAAssetName().ToString();
		return Name.RemoveFromStart(TEXT("PDA_")) && !Name.IsEmpty() ? FName(*Name) : NAME_None;
	}

	void RefreshCoreAuthoredSystemSettings()
	{
		const auto AuthoredBody = CoreAuthoredSettingsBody.Pin();
		if (!AuthoredBody) return;
		static bool bRefreshing = false;
		if (bRefreshing) return;
		TGuardValue<bool> RefreshGuard(bRefreshing, true);
		AuthoredBody->ClearChildren();
		for (const auto& Entry : CoreNativeSystemSettingsBodies)
		{
			if (const auto Body = Entry.Value.Pin()) Body->ClearChildren();
		}
		CoreAuthoredSettingsGroups.Reset();
		CoreAuthoredDetails.Reset();
		CoreAuthoredConfigurations.Reset();

		auto& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
		TArray<FAssetData> Assets;
		Registry.GetAssetsByClass(UGeneralSystemConfiguration_PDA::StaticClass()->GetClassPathName(), Assets, true);
		Assets.Sort([](const FAssetData& A, const FAssetData& B) { return A.PackageName.LexicalLess(B.PackageName); });
		auto& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		for (const FAssetData& Asset : Assets)
		{
			auto* Configuration = Cast<UGeneralSystemConfiguration_PDA>(Asset.GetAsset());
			if (!Configuration || !Configuration->ProjectSettingsDefaults.IsValid()) continue;
			const FName System = GetCoreSettingsSystem(*Configuration);
			if (System.IsNone()) continue;

			const FInstancedStruct Values = UGorgeousSystemProjectSettings::Resolve(*Configuration);
			const UScriptStruct* Schema = Values.GetScriptStruct();
			const auto Scope = MakeShared<FStructOnScope>(Schema);
			Schema->CopyScriptStruct(Scope->GetStructMemory(), Values.GetMemory());
			FDetailsViewArgs Args;
			ConfigureCoreSettingsView(Args, FName(*Configuration->GetPathName()));
			Args.bAllowSearch = false;
			Args.bShowOptions = false;
			Args.bShowScrollBar = false;
			const auto Details = PropertyEditor.CreateStructureDetailView(Args, FStructureDetailsViewArgs(), nullptr);
			if (IDetailsView* View = Details->GetDetailsView()) InitializeCoreSettingsView(*View);
			Details->SetStructureData(Scope);
			if (IDetailsView* View = Details->GetDetailsView()) CollapseInitialCoreSettingsTree(*View);
			const auto Widget = Details->GetWidget();
			if (!Widget) continue;

			Details->GetOnFinishedChangingPropertiesDelegate().AddWeakLambda(Configuration,
				[Configuration, Scope](const FPropertyChangedEvent&)
				{
					const UScriptStruct* CurrentSchema = Cast<UScriptStruct>(Scope->GetStruct());
					if (!CurrentSchema || !Scope->GetStructMemory()) return;
					FInstancedStruct Edited;
					Edited.InitializeAs(CurrentSchema, Scope->GetStructMemory());
					GetMutableDefault<UGorgeousSystemProjectSettings>()->SaveOverride(*Configuration, Edited);
				});
			CoreAuthoredDetails.Add(Details);
			CoreAuthoredConfigurations.Emplace(Configuration);

			const auto* NativeBody = CoreNativeSystemSettingsBodies.Find(System);
			const auto Destination = NativeBody ? NativeBody->Pin() : nullptr;
			if (Destination)
			{
				Destination->AddSlot().AutoHeight()[Widget.ToSharedRef()];
			}
			else
			{
				const auto Area = MakeCoreSettingsArea(FText::FromName(System), Widget.ToSharedRef());
				AuthoredBody->AddSlot().AutoHeight().Padding(6.f, 3.f)[Area];
				CoreAuthoredSettingsGroups.Add(System, Area);
			}
		}
	}

	void RefreshCoreAuthoredSettingsForAsset(const FAssetData& Asset)
	{
		if (!FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry").Get().IsLoadingAssets()
			&& Asset.IsInstanceOf<UGeneralSystemConfiguration_PDA>())
		{
			RefreshCoreAuthoredSystemSettings();
		}
	}

	FIsPropertyVisible UsefulSettingsVisibility(bool bDebuggingPage)
	{
		return FIsPropertyVisible::CreateLambda([bDebuggingPage](const FPropertyAndParent& Entry)
		{
			const FProperty& TopLevel = Entry.ParentProperties.IsEmpty() ? Entry.Property : *Entry.ParentProperties.Last();
			const FString& Category = TopLevel.GetMetaData(TEXT("Category"));
			const bool bDebugging = Category == TEXT("Logging") || Category == TEXT("Blueprint Debugging")
				|| Category == TEXT("Extension Resource Protection");
			return bDebugging == bDebuggingPage;
		});
	}

	void OpenCoreSystemSettings(FName System)
	{
		auto* Settings = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
		const TSharedPtr<SScrollBox> Page = CoreSystemsScrollBox.Pin();
		if (!Settings || !Page) return;

		TArray<TWeakPtr<SExpandableArea>> Groups;
		CoreSystemSettingsGroups.MultiFind(System, Groups);
		CoreAuthoredSettingsGroups.MultiFind(System, Groups);
		TSharedPtr<SExpandableArea> FirstGroup;
		for (const auto& WeakGroup : Groups)
		{
			if (const auto Group = WeakGroup.Pin())
			{
				Group->SetExpanded(true);
				if (!FirstGroup) FirstGroup = Group;
			}
		}
		if (FirstGroup)
		{
			Settings->ShowViewer(CoreProjectContainer, CoreSettingsCategory, CoreSystemsPage);
			Page->ScrollDescendantIntoView(FirstGroup, false, EDescendantScrollDestination::TopOrLeft);
		}
	}

	void AddCoreSystemSettingsAssetAction(UToolMenu* Menu)
	{
		const auto* Context = Menu->FindContext<UContentBrowserAssetContextMenuContext>();
		if (!Context || Context->SelectedAssets.Num() != 1) return;

		const FAssetData& Asset = Context->SelectedAssets[0];
		FName System;
		if (Asset.IsInstanceOf<UGorgeousExtensionResourceGuard>())
		{
			if (const auto* Guard = Cast<UGorgeousExtensionResourceGuard>(Asset.GetAsset()))
			{
				System = Guard->GetGuardSystemIdentifier();
			}
		}
		else if (Asset.IsInstanceOf<UGeneralSystemConfiguration_PDA>())
		{
			if (const auto* Configuration = Cast<UGeneralSystemConfiguration_PDA>(Asset.GetAsset()))
			{
				System = GetCoreSettingsSystem(*Configuration);
			}
		}
		if (System.IsNone() || (!CoreSystemSettingsGroups.Contains(System) && !CoreAuthoredSettingsGroups.Contains(System))) return;

		Menu->FindOrAddSection("GorgeousSystemSettings").AddMenuEntry(
			"OpenGorgeousSystemSettings",
			LOCTEXT("OpenSystemSettings", "Open System Project Settings"),
			LOCTEXT("OpenSystemSettingsTooltip", "Open the settings for this Gorgeous system."),
			FSlateIcon(), FUIAction(FExecuteAction::CreateStatic(&OpenCoreSystemSettings, System)));
	}

#if GORGEOUS_58_SWITCH(0, 1)
	void OpenGorgeousCoreSettingsPage(FName Section)
	{
		if (auto* Settings = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			Settings->ShowViewer(CoreProjectContainer, CoreSettingsCategory, Section);
		}
	}

	void FillGorgeousSettingsShortcut(UToolMenu* Menu)
	{
		FToolMenuSection& Section = Menu->AddSection("GorgeousSettingsPages");
		const auto AddPage = [&Section](FName Id, const FText& Label)
		{
			Section.AddMenuEntry(Id, Label, FText::GetEmpty(), FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&OpenGorgeousCoreSettingsPage, Id)));
		};
		AddPage(CoreGeneralPage, LOCTEXT("ShortcutGeneral", "General"));
		AddPage(CoreAutoReplicationPage, LOCTEXT("ShortcutAutoReplication", "Auto Replication"));
		AddPage(CoreDebuggingPage, LOCTEXT("ShortcutDebugging", "Debugging"));
		AddPage(CoreSystemsPage, LOCTEXT("ShortcutSystems", "Systems"));
	}
#endif
}

//=============================================================================
// GorgeousCoreSettingsPages Implementation
//=============================================================================

void GorgeousCoreSettingsPages::Register(ISettingsModule& Settings)
{
	const auto RegisterPage = [&Settings](FName Id, const FText& Title, const FText& Description,
		const TSharedRef<SScrollBox>& Page)
	{
		CoreSettingsWidgets.Add(Id, Page);
		Settings.RegisterSettings(CoreProjectContainer, CoreSettingsCategory, Id, Title, Description, Page);
	};

	const TSharedRef<SScrollBox> General = SNew(SScrollBox);
	const TSharedRef<SScrollBox> Debugging = SNew(SScrollBox);
	const TSharedRef<SScrollBox> Systems = SNew(SScrollBox);
	const TSharedRef<SScrollBox> AutoReplication = SNew(SScrollBox);
	CoreSystemsScrollBox = Systems;

	struct FSettingsGroup
	{
		UGorgeousCoreDeveloperSettings* Object;
		FText Title;
	};
	TArray<FSettingsGroup> Groups;
	for (TObjectIterator<UClass> Class; Class; ++Class)
	{
		if (!Class->IsChildOf(UGorgeousCoreDeveloperSettings::StaticClass())
			|| Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists)
			|| Class->GetMetaData(TEXT("GorgeousSettingsPage")) == TEXT("Hidden"))
		{
			continue;
		}

		const FString& Guard = Class->GetMetaData(TEXT("GorgeousSettingsGuard"));
		if (!Guard.IsEmpty() && !GorgeousCompiledSystemGuards::IsAvailable(FName(*Guard)))
		{
			continue;
		}

		auto* Object = Class->GetDefaultObject<UGorgeousCoreDeveloperSettings>();
		FText Title = Object->GetSectionText();
		FString Label = Title.ToString();
		if (Label.RemoveFromStart(TEXT("Core ↪ ")))
		{
			Title = FText::FromString(Label);
		}
		Groups.Add({Object, MoveTemp(Title)});
	}

	Groups.Sort([](const FSettingsGroup& A, const FSettingsGroup& B) { return A.Title.CompareTo(B.Title) < 0; });
	for (const FSettingsGroup& Group : Groups)
	{
		UClass* Class = Group.Object->GetClass();
		const FString& Page = Class->GetMetaData(TEXT("GorgeousSettingsPage"));
		SScrollBox& Destination = Page == TEXT("Systems") ? *Systems
			: Page == TEXT("General") ? *General
			: Page == TEXT("AutoReplication") ? *AutoReplication : *Debugging;
		const bool bSplitDiagnostics = Class->HasMetaData(TEXT("GorgeousSplitEditorDiagnostics"));
		FName SystemName;
		if (Page == TEXT("Systems"))
		{
			const FString& System = Class->HasMetaData(TEXT("GorgeousSettingsSystem"))
				? Class->GetMetaData(TEXT("GorgeousSettingsSystem"))
				: Class->GetMetaData(TEXT("GorgeousSettingsGuard"));
			if (!System.IsEmpty()) SystemName = FName(*System);
		}
		const auto Area = AddCoreSettingsGroup(Destination, Class->GetFName(), Group.Title, *Group.Object,
			bSplitDiagnostics ? UsefulSettingsVisibility(false) : FIsPropertyVisible(), SystemName);
		if (!SystemName.IsNone()) CoreSystemSettingsGroups.Add(SystemName, Area);
		if (bSplitDiagnostics)
		{
			AddCoreSettingsGroup(*Debugging, GorgeousCoreSettingsPages::EditorDiagnosticsViewId, LOCTEXT("EditorDiagnostics", "Editor Diagnostics"),
				*Group.Object, UsefulSettingsVisibility(true));
		}
	}

	RegisterPage(CoreGeneralPage, LOCTEXT("GeneralPage", "Core ↪ General"),
		LOCTEXT("GeneralDescription", "Library, privacy, development tools and editor preferences."), General);
	RegisterPage(CoreAutoReplicationPage, LOCTEXT("AutoReplicationPage", "Core ↪ Auto Replication"),
		LOCTEXT("AutoReplicationDescription", "Configure Iris and Replication Graph backends for auto-replicated objects."), AutoReplication);
	RegisterPage(CoreDebuggingPage, LOCTEXT("DebuggingPage", "Core ↪ Debugging"),
		LOCTEXT("DebuggingDescription", "Core runtime configuration, logging, diagnostics and Insight Matrix charts."), Debugging);
	RegisterPage(CoreSystemsPage, LOCTEXT("SystemsPage", "Core ↪ Systems"),
		LOCTEXT("SystemsDescription", "Settings for the Core systems used by this project."), Systems);

	const auto AuthoredBody = SNew(SVerticalBox);
	CoreAuthoredSettingsBody = AuthoredBody;
	Systems->AddSlot()[AuthoredBody];
	auto& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	CoreSettingsFilesLoaded = Registry.OnFilesLoaded().AddStatic(&RefreshCoreAuthoredSystemSettings);
	CoreSettingsAssetAdded = Registry.OnAssetAdded().AddStatic(&RefreshCoreAuthoredSettingsForAsset);
	CoreSettingsAssetRemoved = Registry.OnAssetRemoved().AddStatic(&RefreshCoreAuthoredSettingsForAsset);
	CoreSettingsAssetUpdated = Registry.OnAssetUpdated().AddStatic(&RefreshCoreAuthoredSettingsForAsset);
	CoreSettingsAssetRenamed = Registry.OnAssetRenamed().AddLambda(
		[](const FAssetData& Asset, const FString&) { RefreshCoreAuthoredSettingsForAsset(Asset); });
	if (!Registry.IsLoadingAssets()) RefreshCoreAuthoredSystemSettings();

	if (UToolMenus::IsToolMenuUIEnabled())
	{
		FToolMenuOwnerScoped Owner(CoreSystemSettingsMenuOwner);
		UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu")->AddDynamicSection(
			"GorgeousSystemSettingsDynamic", FNewToolMenuDelegate::CreateStatic(&AddCoreSystemSettingsAssetAction));
#if GORGEOUS_58_SWITCH(0, 1)
		UToolMenu* Toolbar = UToolMenus::Get()->ExtendMenu("SettingsEditor.SearchToolbar");
		Toolbar->FindOrAddSection("GorgeousSettings").AddEntry(FToolMenuEntry::InitComboButton(
			"GorgeousThingsSettings",
			FUIAction(),
			FNewToolMenuDelegate::CreateStatic(&FillGorgeousSettingsShortcut),
			LOCTEXT("SettingsShortcut", "Gorgeous Things"),
			LOCTEXT("SettingsShortcutTooltip", "Open Gorgeous Things project settings."),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Settings")))
			.SetShowInToolbarTopLevel(true);
#endif
	}
}

void GorgeousCoreSettingsPages::Unregister(ISettingsModule* Settings)
{
	if (UToolMenus::IsToolMenuUIEnabled()) UToolMenus::UnregisterOwner(CoreSystemSettingsMenuOwner);
	if (auto* AssetRegistry = FModuleManager::GetModulePtr<FAssetRegistryModule>("AssetRegistry"))
	{
		auto& Registry = AssetRegistry->Get();
		Registry.OnFilesLoaded().Remove(CoreSettingsFilesLoaded);
		Registry.OnAssetAdded().Remove(CoreSettingsAssetAdded);
		Registry.OnAssetRemoved().Remove(CoreSettingsAssetRemoved);
		Registry.OnAssetUpdated().Remove(CoreSettingsAssetUpdated);
		Registry.OnAssetRenamed().Remove(CoreSettingsAssetRenamed);
	}
	CoreSystemSettingsGroups.Reset();
	CoreAuthoredSettingsGroups.Reset();
	CoreNativeSystemSettingsBodies.Reset();
	CoreAuthoredSettingsBody.Reset();
	CoreSystemsScrollBox.Reset();
	if (Settings)
	{
		for (const auto& Page : CoreSettingsWidgets)
		{
			Settings->UnregisterSettings(CoreProjectContainer, CoreSettingsCategory, Page.Key);
		}
	}
	CoreSettingsWidgets.Reset();
	CoreAuthoredDetails.Reset();
	CoreAuthoredConfigurations.Reset();
}

#undef LOCTEXT_NAMESPACE