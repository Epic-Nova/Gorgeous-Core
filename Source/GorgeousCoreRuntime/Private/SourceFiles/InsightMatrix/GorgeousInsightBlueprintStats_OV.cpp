// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Insight Blueprint Stats OV                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/

#include "InsightMatrix/GorgeousInsightBlueprintStats_OV.h"

#include "GorgeousInsightActionConfig_DA.h"
#include "ObjectVariables/GorgeousObjectVariableRegistry_GIS.h"
#include "ObjectVariables/GorgeousRootObjectVariable.h"
#include "Kismet/GameplayStatics.h"

FGuid UGorgeousInsightBlueprintStats_OV::RegisterBlueprintSystemStats(UObject* WorldContextObject, FName SystemName)
{
	// Try to find an existing registry OV globally
	TArray<UGorgeousObjectVariable*> FoundOVs = UGorgeousRootObjectVariable::GetVariableHierarchyRegistry();
	
	UGorgeousInsightBlueprintStats_OV* StatsRegistry = nullptr;
	for (UGorgeousObjectVariable* OV : FoundOVs)
	{
		if (UGorgeousInsightBlueprintStats_OV* StatsOV = Cast<UGorgeousInsightBlueprintStats_OV>(OV))
		{
			StatsRegistry = StatsOV;
			break;
		}
	}

	if (!StatsRegistry)
	{
		// Create it if it doesn't exist
		UGorgeousRootObjectVariable* Root = UGorgeousRootObjectVariable::GetRootObjectVariable();
		if (Root)
		{
			StatsRegistry = Cast<UGorgeousInsightBlueprintStats_OV>(Root->InstantiateTransactionalObjectVariable(UGorgeousInsightBlueprintStats_OV::StaticClass(), Root));
			if (StatsRegistry)
			{
				Root->RegisterWithRegistry(StatsRegistry);
			}
		}
	}

	if (StatsRegistry)
	{
		// Ensure the system map entry exists
		if (!StatsRegistry->SystemStatsMap.Contains(SystemName))
		{
			StatsRegistry->SystemStatsMap.Add(SystemName, FGorgeousBlueprintSystemStatsData());
			StatsRegistry->MarkPackageDirty();
		}
		
		return StatsRegistry->UniqueIdentifier;
	}

	return FGuid();
}

void UGorgeousInsightBlueprintStats_OV::RegisterBlueprintSystemAction(UObject* WorldContextObject, const FGuid& RegistryGuid, FName SystemName, FName ActionName, FGameplayTag SignalBridgeTag)
{
	if (!RegistryGuid.IsValid() || !WorldContextObject)
	{
		return;
	}

	if (UGorgeousObjectVariable* OV = UGorgeousRootObjectVariable::FindVariableByIdentifier(RegistryGuid))
	{
		if (UGorgeousInsightBlueprintStats_OV* StatsRegistry = Cast<UGorgeousInsightBlueprintStats_OV>(OV))
		{
			FGorgeousBlueprintSystemStatsData& Data = StatsRegistry->SystemStatsMap.FindOrAdd(SystemName);
			
			// Check if action already exists
			bool bExists = false;
			for (const FGorgeousBlueprintStatsInsightAction& Action : Data.Actions)
			{
				if (Action.ActionName == ActionName)
				{
					bExists = true;
					break;
				}
			}

			if (!bExists)
			{
				FGorgeousBlueprintStatsInsightAction NewAction;
				NewAction.ActionName = ActionName;
				NewAction.SignalBridgeTag = SignalBridgeTag;
				Data.Actions.Add(NewAction);
				StatsRegistry->MarkPackageDirty();
			}
		}
	}
}

void UGorgeousInsightBlueprintStats_OV::UnregisterBlueprintSystemAction(UObject* WorldContextObject, const FGuid& RegistryGuid, FName SystemName, FName ActionName)
{
	if (!RegistryGuid.IsValid() || !WorldContextObject)
	{
		return;
	}

	if (UGorgeousObjectVariable* OV = UGorgeousRootObjectVariable::FindVariableByIdentifier(RegistryGuid))
	{
		if (UGorgeousInsightBlueprintStats_OV* StatsRegistry = Cast<UGorgeousInsightBlueprintStats_OV>(OV))
		{
			if (FGorgeousBlueprintSystemStatsData* Data = StatsRegistry->SystemStatsMap.Find(SystemName))
			{
				for (int32 i = Data->Actions.Num() - 1; i >= 0; --i)
				{
					if (Data->Actions[i].ActionName == ActionName)
					{
						Data->Actions.RemoveAt(i);
						StatsRegistry->MarkPackageDirty();
						break;
					}
				}
			}
		}
	}
}

void UGorgeousInsightBlueprintStats_OV::UnregisterBlueprintSystemStats(UObject* WorldContextObject, const FGuid& RegistryGuid, FName SystemName)
{
	if (!RegistryGuid.IsValid() || !WorldContextObject)
	{
		return;
	}

	if (UGorgeousObjectVariable* OV = UGorgeousRootObjectVariable::FindVariableByIdentifier(RegistryGuid))
	{
		if (UGorgeousInsightBlueprintStats_OV* StatsRegistry = Cast<UGorgeousInsightBlueprintStats_OV>(OV))
		{
			if (StatsRegistry->SystemStatsMap.Remove(SystemName) > 0)
			{
				StatsRegistry->MarkPackageDirty();
			}
		}
	}
}

DEFINE_FUNCTION(UGorgeousInsightBlueprintStats_OV::execSetBlueprintSystemStat)
{
	P_GET_STRUCT_REF(FGuid, RegistryGuid);
	P_GET_PROPERTY(FNameProperty, SystemName);
	P_GET_PROPERTY(FStrProperty, StatName);

	Stack.StepCompiledIn<FProperty>(NULL);
	void* ValueAddr = Stack.MostRecentPropertyAddress;
	FProperty* ValueProperty = Stack.MostRecentProperty;

	P_FINISH;

	P_NATIVE_BEGIN;
	
	if (RegistryGuid.IsValid() && ValueProperty && ValueAddr)
	{
		if (UGorgeousObjectVariable* OV = UGorgeousRootObjectVariable::FindVariableByIdentifier(RegistryGuid))
		{
			if (UGorgeousInsightBlueprintStats_OV* StatsRegistry = Cast<UGorgeousInsightBlueprintStats_OV>(OV))
			{
				double NumericValue = 0.0;
				bool bValidNumber = false;

				if (const FNumericProperty* NumericProp = CastField<FNumericProperty>(ValueProperty))
				{
					if (NumericProp->IsFloatingPoint())
					{
						NumericValue = NumericProp->GetFloatingPointPropertyValue(ValueAddr);
					}
					else if (NumericProp->IsInteger())
					{
						NumericValue = static_cast<double>(NumericProp->GetSignedIntPropertyValue(ValueAddr));
					}
					bValidNumber = true;
				}
				else if (const FBoolProperty* BoolProp = CastField<FBoolProperty>(ValueProperty))
				{
					NumericValue = BoolProp->GetPropertyValue(ValueAddr) ? 1.0 : 0.0;
					bValidNumber = true;
				}

				if (bValidNumber)
				{
					FGorgeousBlueprintSystemStatsData& Data = StatsRegistry->SystemStatsMap.FindOrAdd(SystemName);
					Data.NumericStats.Add(StatName, NumericValue);
					StatsRegistry->MarkPackageDirty();
				}
			}
		}
	}

	P_NATIVE_END;
}
