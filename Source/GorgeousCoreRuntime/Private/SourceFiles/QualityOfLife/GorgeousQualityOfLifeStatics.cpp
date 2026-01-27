// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "QualityOfLife/GorgeousQualityOfLifeStatics.h"

#include "AutoReplication/GorgeousAutoReplicationMixin.h"
#include "ModuleCore/GorgeousCoreRuntimeGlobals.h"
#include "ModuleCore/GorgeousObjectVariableRootSettings.h"
#include "ObjectVariables/NativeObjectVariableDefinitions.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "Engine/GameInstance.h"

namespace
{
	static const FName DefaultLocalRootName(TEXT("DefaultRoot"));
	static const FName DefaultNetworkRootName(TEXT("DefaultNetworkRoot"));

	FName ResolvePreferredRootName(const bool bWantsNetworking)
	{
		if (const UGorgeousObjectVariableRootSettings* Settings = UGorgeousObjectVariableRootSettings::Get())
		{
			if (const FName DesiredName = bWantsNetworking ? DefaultNetworkRootName : DefaultLocalRootName; Settings->FindRootEntry(DesiredName))
			{
				return DesiredName;
			}

			if (bWantsNetworking)
			{
				for (const FGorgeousObjectVariableRootEntry& Entry : Settings->RegisteredRoots)
				{
					if (Entry.bSupportsNetworking && !Entry.bHandlesParentlessReplicatedVariables)
					{
						return Entry.RootName;
					}
				}
			}
			else
			{
				return Settings->GetDefaultRootName();
			}
		}

		return bWantsNetworking ? DefaultNetworkRootName : UGorgeousRootObjectVariable::GetDefaultRootName();
	}

	FName MakeSubobjectName(const UObject* Owner)
	{
		return FName(*FString::Printf(TEXT("%s_SelfReferenceOV"), *Owner->GetClass()->GetName()));
	}

}

namespace FGorgeousQualityOfLifeStatics
{
	void SanitizeCDOAdditionalData(UObject* Owner, TMap<FName, FGorgeousAutoReplicationEntry>& AdditionalData)
	{
		if (!Owner || !Owner->HasAnyFlags(RF_ClassDefaultObject))
		{
			return;
		}

		if (FGorgeousAutoReplicationEntry* SelfEntry = AdditionalData.Find("SelfReference"))
		{
			if (SelfEntry->DefaultValue)
			{
				UGorgeousRootObjectVariable::RemoveVariableFromRegistry(SelfEntry->DefaultValue);
				SelfEntry->DefaultValue = nullptr;
			}
			SelfEntry->bReplicate = false;
			SelfEntry->Handle = FGorgeousAutoReplicationHandle();
			AdditionalData.Remove("SelfReference");
		}
	}

	UObject_AOTOV* EnsureSelfReference(UObject* Owner, TMap<FName, FGorgeousAutoReplicationEntry>& AdditionalData, const bool bExposeThroughNetworkStack)
	{
		if (!Owner)
		{
			return nullptr;
		}

		// Do not instantiate or persist self-reference entries on class default objects; they should only exist at runtime.
		if (Owner->HasAnyFlags(RF_ClassDefaultObject))
		{
			SanitizeCDOAdditionalData(Owner, AdditionalData);
			return nullptr;
		}


		//TODO: Comply with splitscreen logic. Currently we work with one self reference per player index, we want to have one self reference per game instance, witch results in an oobject array that contains the reference to the local index objects
		FGorgeousAutoReplicationEntry& Entry = AdditionalData.FindOrAdd("SelfReference");
		UObject_AOTOV* SelfVariable = Cast<UObject_AOTOV>(Entry.DefaultValue);
		if (!SelfVariable || SelfVariable->GetOuter() != Owner)
		{
			EObjectFlags SelfReferenceFlags = RF_Transactional;
			SelfVariable = NewObject<UObject_AOTOV>(Owner, MakeSubobjectName(Owner), SelfReferenceFlags);
			Entry.DefaultValue = SelfVariable;
		}

		if (SelfVariable && !SelfVariable->UniqueIdentifier.IsValid())
		{
			SelfVariable->UniqueIdentifier = FGuid::NewGuid();
		}
		// Rooted parent (GameInstance) holds this variable; root the OV as well to satisfy GC when referenced by a root-set registry.
		if (SelfVariable && !SelfVariable->IsRooted())
		{
			SelfVariable->AddToRoot();
		}
		if (!SelfVariable)
		{
			return nullptr;
		}

		const bool bIsClassDefaultObject = Owner->HasAnyFlags(RF_ClassDefaultObject);

		// GameInstance is long-lived but not rooted by default; AddToRoot avoids GC-verify warnings when a root OV holds it.
		if (!bIsClassDefaultObject && Owner->IsA<UGameInstance>() && !Owner->IsRooted())
		{
			Owner->AddToRoot();
		}

		const FName PreferredRoot = ResolvePreferredRootName(bExposeThroughNetworkStack);
		SelfVariable->RootConfiguration.PreferredRootName = PreferredRoot;
		SelfVariable->RootNetworkConfig.bExposeThroughRootNetworkStack = bExposeThroughNetworkStack;
		SelfVariable->RootNetworkConfig.AccessPolicy = EGorgeousObjectVariableAccessPolicy::Everyone;
		SelfVariable->RootNetworkConfig.ReplicationChannel = "SelfReference";
		SelfVariable->SetNetworkingEnabled(bExposeThroughNetworkStack);

		if (!bIsClassDefaultObject && !SelfVariable->GetParent())
		{
			if (UGorgeousRootObjectVariable* Root = UGorgeousRootObjectVariable::GetRootObjectVariable(PreferredRoot))
			{
				SelfVariable->SetParent(Root);
			}
		}

		if (!bIsClassDefaultObject)
		{
			FString ContextName = FString::Printf(TEXT("SelfReference_%s"), *Owner->GetClass()->GetName());
			if (int32 UnderscoreIndex; ContextName.FindLastChar('_', UnderscoreIndex)) { ContextName = ContextName.Left(UnderscoreIndex); }
			SelfVariable->SetDisplayName(ContextName);
		}
		Entry.bReplicate = bExposeThroughNetworkStack;

		if (bIsClassDefaultObject || !Entry.bOverrideStreamConfig)
		{
			Entry.StreamConfigOverride = UGorgeousCoreRuntimeGlobals::GetDefaultAutoReplicationStreamConfig();
			Entry.StreamConfigOverride.bRespectAccessPolicy = Entry.StreamConfigOverride.bRespectAccessPolicy || bExposeThroughNetworkStack;
			Entry.bOverrideStreamConfig = true;
		}

		if (!bIsClassDefaultObject)
		{
			UObject* OwnerReference = Owner;

			TArray<UObject*> OwnerArray = IGorgeousArrayObjectVariablesGetter_I::Execute_GetObjectObjectArrayObjectVariable(SelfVariable, NAME_None);

			if (OwnerArray.Contains(OwnerReference))
			{
				OwnerArray[OwnerArray.IndexOfByKey(OwnerReference)] = OwnerReference;
			}
			else
			{
				OwnerArray.Add(OwnerReference);
			}
			
			IGorgeousArrayObjectVariablesSetter_I::Execute_SetObjectObjectArrayObjectVariable(SelfVariable, NAME_None, OwnerArray);
		}

		return SelfVariable;
	}

	void ClearSelfReference(const UObject* Owner, TMap<FName, FGorgeousAutoReplicationEntry>& AdditionalData)
	{
		if (!Owner)
		{
			return;
		}
		
		if (const FGorgeousAutoReplicationEntry* Entry = AdditionalData.Find("SelfReference"))
		{
			if (Entry->DefaultValue)
			{
				UGorgeousRootObjectVariable::RemoveVariableFromRegistry(Entry->DefaultValue);
			}
			AdditionalData.Remove("SelfReference");
		}
	}

	UObject* ResolveSelfReference(const TSubclassOf<UObject> QualityOfLifeClass)
	{
		return UGorgeousCoreRuntimeGlobals::GetNamedObjectReference(FName(ResolveSelfReferenceName(QualityOfLifeClass)), false);
	}

	FString ResolveSelfReferenceName(const TSubclassOf<UObject> QualityOfLifeClass)
	{
		//TODO: Current approach is fixing the stroke where we have multiple of these self references registered. or they have wierd namings whitch makes them impossible to address via a expected name without _X at the end.
		////In the best case we expect only one of a kind self reference inside the root registry across the whole game instance
		UGorgeousRootObjectVariable* Roots[] = {
			UGorgeousRootObjectVariable::GetRootObjectVariable(ResolvePreferredRootName(false)),
			UGorgeousRootObjectVariable::GetRootObjectVariable(ResolvePreferredRootName(true))
		};

		for (UGorgeousRootObjectVariable* Root : Roots)
		{
			if (!Root) { continue; }
			for (const auto Entry : Root->VariableRegistry)
			{
				if (Entry && Entry->GetDisplayName().Contains("SelfReference"))
				{
					if (const UObject_AOTOV* SelfRefOV = Cast<UObject_AOTOV>(Entry))
					{
						for (const TArray<UObject*> ReferencedObject = IGorgeousArrayObjectVariablesGetter_I::Execute_GetObjectObjectArrayObjectVariable(SelfRefOV, NAME_None);
							const auto Object : ReferencedObject)
						{
							if (!Object || !Object->IsA(*QualityOfLifeClass)) { continue; }

							return Entry->GetDisplayName();
						}
					}
				}
			}
		}

		return LexToString(NAME_None);
	}
}
