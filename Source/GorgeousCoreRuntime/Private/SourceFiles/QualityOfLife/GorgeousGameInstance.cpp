// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
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

UGorgeousGameInstance::UGorgeousGameInstance()
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FGorgeousQualityOfLifeStatics::SanitizeCDOAdditionalData(this, AdditionalGorgeousData);
	}
	FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, false);
}

void UGorgeousGameInstance::Init()
{
	FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, false);

	if (!UGorgeousRootObjectVariable::GetRootObjectVariable())
	{
		UE_LOG(LogGorgeousGameInstance, Error, TEXT("RootObjectVariable is null during Init."));
		return;
	}

	EnsureRootVariablesFallbackToGameInstance();

	for (auto& GorgeousSetting : AdditionalGorgeousData)
	{
		if (UGorgeousObjectVariable* const DefaultVar = GorgeousSetting.Value.DefaultValue)
		{
			if (!DefaultVar->UniqueIdentifier.IsValid())
			{
				DefaultVar->UniqueIdentifier = FGuid::NewGuid();
			}
			DefaultVar->InvokeInstancedFunctionality(DefaultVar->UniqueIdentifier);
		}
	}

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

	//@TODO: Somehow leads to a crash without any log & crash context
	/*for (const FName& RootName : RootsToInitialize)
	{
		const bool bSupportsSharedOwnership = UGorgeousRootObjectVariable::IsSharedNetworkingRoot(RootName);
		if (UGorgeousRootObjectVariable* Root = UGorgeousRootObjectVariable::GetRootObjectVariable(RootName))
		{
			Root->SetFallbackOwner(this);

			// Re-outer roots that were created before any world existed
			// (still on GetTransientPackage) so UObject::GetWorld() works.
			if (Root->GetOuter() == GetTransientPackage())
			{
				Root->Rename(nullptr, this);
			}

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
	}*/

	for (const TPair<FName, TObjectPtr<UGorgeousRootObjectVariable>>& Pair : UGorgeousRootObjectVariable::NamedRootInstances)
	{
		if (UGorgeousRootObjectVariable* RootInstance = Pair.Value)
		{
			RootInstance->SetFallbackOwner(this);

			if (RootInstance->GetOuter() == GetTransientPackage())
			{
				RootInstance->Rename(nullptr, this);
			}
		}
	}
}

void UGorgeousGameInstance::PostInitProperties()
{
	Super::PostInitProperties();
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FGorgeousQualityOfLifeStatics::SanitizeCDOAdditionalData(this, AdditionalGorgeousData);
		return;
	}
	FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, false);
}

void UGorgeousGameInstance::PostLoad()
{
	Super::PostLoad();
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FGorgeousQualityOfLifeStatics::SanitizeCDOAdditionalData(this, AdditionalGorgeousData);
		return;
	}
	FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, false);
}

void UGorgeousGameInstance::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UGorgeousGameInstance, AdditionalGorgeousData))
	{
		FGorgeousQualityOfLifeStatics::EnsureSelfReference(this, AdditionalGorgeousData, false);
		for (auto& GorgeousSetting : AdditionalGorgeousData)
		{
			if (UGorgeousObjectVariable* const DefaultVar = GorgeousSetting.Value.DefaultValue)
			{
				DefaultVar->UniqueIdentifier = FGuid::NewGuid();
			}
		}
	}
}

void UGorgeousGameInstance::HandleRootRegistryChanged()
{
	bIsEnsuringRootFallback = false;
	EnsureRootVariablesFallbackToGameInstance();
}
