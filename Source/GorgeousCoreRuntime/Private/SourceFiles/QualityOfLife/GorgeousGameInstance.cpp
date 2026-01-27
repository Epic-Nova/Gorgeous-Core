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
#include "QualityOfLife/GorgeousGameInstance.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "QualityOfLife/GorgeousQualityOfLifeStatics.h"
#include "QualityOfLife/GorgeousQualityOfLifeHelperMacros.h"
#include "AutoReplication/Helpers/GorgeousAutoReplicationHelperMacros.h"
#include "Containers/Set.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/GameEngine.h"
#include "Templates/UnrealTemplate.h"
//<-------------------------------------------------------------------------->

//=============================================================================
// UGorgeousGameInstance Implementation
//=============================================================================

DEFINE_LOG_CATEGORY_STATIC(LogGorgeousGameInstance, Log, All);

UE_QOL_DEFINE_CONSTRUCTOR(UGorgeousGameInstance, false)

UE_QOL_DEFINE_HANDLE_AUTOREPLICATION_RPC(UGorgeousGameInstance)

UE_QOL_DEFINE_POST_EDIT_CHANGE_PROPERTY(UGorgeousGameInstance)

void UGorgeousGameInstance::Init()
{
	FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, bActivateNetworkingCapabilities);

	if (!UGorgeousRootObjectVariable::GetRootObjectVariable())
	{
		UE_LOG(LogGorgeousGameInstance, Error, TEXT("RootObjectVariable is null during Init."));
		return;
	}

	EnsureRootVariablesFallbackToGameInstance();

	UE_DECLARE_AUTOREPLICATION_CLASS_INIT_INVOKE_ADDITIONAL_DATA

	if (!RootRegistryChangedHandle.IsValid())
	{
		RootRegistryChangedHandle = UGorgeousRootObjectVariable::OnRootRegistryChanged.AddUObject(this, &UGorgeousGameInstance::HandleRootRegistryChanged);
	}

	Super::Init();
}

void UGorgeousGameInstance::Shutdown()
{
	if (RootRegistryChangedHandle.IsValid())
	{
		UGorgeousRootObjectVariable::OnRootRegistryChanged.Remove(RootRegistryChangedHandle);
		RootRegistryChangedHandle.Reset();
	}

	Super::Shutdown();
}

void UGorgeousGameInstance::EnsureRootVariablesFallbackToGameInstance()
{
	if (bIsEnsuringRootFallback)
	{
		return;
	}

	TGuardValue<bool> ReentrancyGuard(bIsEnsuringRootFallback, true);

	TSet<FName> RootsToInitialize;
	const FName DefaultRootName = UGorgeousRootObjectVariable::GetDefaultRootName();
	RootsToInitialize.Add(DefaultRootName.IsNone() ? NAME_None : DefaultRootName);

	const TArray<FName> RegisteredRootNames = UGorgeousRootObjectVariable::GetRegisteredRootNames();
	for (const FName& RootName : RegisteredRootNames)
	{
		if (!RootName.IsNone())
		{
			RootsToInitialize.Add(RootName);
		}
	}

	for (const FName& RootName : RootsToInitialize)
	{
		const bool bSupportsSharedOwnership = UGorgeousRootObjectVariable::IsSharedNetworkingRoot(RootName);
		if (UGorgeousRootObjectVariable* Root = UGorgeousRootObjectVariable::GetRootObjectVariable(RootName))
		{
			Root->SetCachedOwner(this);
			if (bSupportsSharedOwnership)
			{
				Root->EnsureSharedNetworkStackOwner(this);
				UGorgeousRootObjectVariable::PromoteRootRegistryOwner(RootName, this);
			}
		}
		else
		{
			const FString Label = RootName.IsNone() ? TEXT("<DefaultRoot>") : RootName.ToString();
			UE_LOG(LogGorgeousGameInstance, Warning, TEXT("Failed to assign cached owner for root object variable '%s'."), *Label);
		}
	}

	for (const TPair<FName, TObjectPtr<UGorgeousRootObjectVariable>>& Pair : UGorgeousRootObjectVariable::NamedRootInstances)
	{
		if (UGorgeousRootObjectVariable* RootInstance = Pair.Value)
		{
			RootInstance->SetCachedOwner(this);
		}
	}
}

UE_QOL_DEFINE_POST_INIT_AND_LOAD(UGorgeousGameInstance)

void UGorgeousGameInstance::HandleRootRegistryChanged()
{
	EnsureRootVariablesFallbackToGameInstance();
}
