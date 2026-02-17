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
#include "GorgeousCoreEditorModule.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GorgeousCoreMinimalShared.h"
#include "InsightMatrix/GorgeousInsightMatrixSubsystem.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "BlueprintActionDatabase.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphUtilities.h"
#include "Editor.h"
#include "GorgeousRainbowPinFactory.h"
#include "IMessageLogListing.h"
#include "MessageLogModule.h"
#include "K2Nodes/GorgeousK2Node_Get.h"
#include "K2Nodes/GorgeousK2Node_Set.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// FGorgeousCoreEditorModule Implementation
//=============================================================================

namespace
{
	FDelegateHandle BeginPIEHandle;
	FDelegateHandle EndPIEHandle;
	TSharedPtr<FRainbowPinFactory> Factory;
	TSharedPtr<FGorgeousRainbowGraphPinFactory> RainbowPinFactory;
	TSharedPtr<FGorgeousRainbowConnectionFactory> RainbowConnectionFactory;

	void HandleBeginPIE(bool bIsSimulating)
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
		}
	}

	void HandleEndPIE(bool bIsSimulating)
	{
		if (UGorgeousInsightMatrixSubsystem* Subsystem = UGorgeousInsightMatrixSubsystem::Get())
		{
			Subsystem->HideInGamePanel();
		}
	}
}

void FGorgeousCoreEditorModule::GorgeousStartupModule()
{
	Factory = MakeShareable(new FRainbowPinFactory());
	FEdGraphUtilities::RegisterVisualNodeFactory(Factory);

	RainbowPinFactory = MakeShareable(new FGorgeousRainbowGraphPinFactory());
	FEdGraphUtilities::RegisterVisualPinFactory(RainbowPinFactory);

	RainbowConnectionFactory = MakeShareable(new FGorgeousRainbowConnectionFactory());
	FEdGraphUtilities::RegisterVisualPinConnectionFactory(RainbowConnectionFactory);
	
	//@TODO: Planned for version 1.1 and upwards
	/*FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(
		UGorgeousObjectVariable::StaticClass()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGorgeousObjectVariablePropertyTypeCustomization::MakeInstance));
	PropertyEditorModule.NotifyCustomizationModuleChanged();*/

	BeginPIEHandle = FEditorDelegates::BeginPIE.AddStatic(&HandleBeginPIE);
	EndPIEHandle = FEditorDelegates::EndPIE.AddStatic(&HandleEndPIE);
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
	
	FEdGraphUtilities::UnregisterVisualNodeFactory(Factory);
	FEdGraphUtilities::UnregisterVisualPinFactory(RainbowPinFactory);
	FEdGraphUtilities::UnregisterVisualPinConnectionFactory(RainbowConnectionFactory);

	//@TODO: Planned for version 1.1 and upwards
	/*if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.UnregisterCustomPropertyTypeLayout(UGorgeousObjectVariable::StaticClass()->GetFName());
	}*/
}

TArray<FName> FGorgeousCoreEditorModule::GetDependentPlugins() const
{
	return TArray<FName>();
}

int32 FGorgeousCoreEditorModule::GetMinimumRequiredCoreVersion() const
{
	//Actually not needed as the Core does not perform checks against itself.
	return 100;
}

IMPLEMENT_MODULE(FGorgeousCoreEditorModule, GorgeousCoreEditor)