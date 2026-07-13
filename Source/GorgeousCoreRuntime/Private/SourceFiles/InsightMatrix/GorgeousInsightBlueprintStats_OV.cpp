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
#include "ObjectVariables/GorgeousObjectVariableRegistry_GIS.h"

FGuid UGorgeousInsightBlueprintStats_OV::RegisterBlueprintSystemStats(UObject* WorldContextObject, FName SystemName)
{
	UGorgeousObjectVariableRegistry_GIS* Registry = UGorgeousObjectVariableRegistry_GIS::Get(WorldContextObject);
	if (!Registry)
	{
		return FGuid();
	}

	// Try to find an existing registry OV globally
	TArray<UGorgeousObjectVariable*> FoundOVs = Registry->GetAllObjectVariablesOfClass(UGorgeousInsightBlueprintStats_OV::StaticClass(), true);
	
	UGorgeousInsightBlueprintStats_OV* StatsRegistry = nullptr;
	if (FoundOVs.Num() > 0)
	{
		StatsRegistry = Cast<UGorgeousInsightBlueprintStats_OV>(FoundOVs[0]);
	}
	else
	{
		// Create it if it doesn't exist
		StatsRegistry = Cast<UGorgeousInsightBlueprintStats_OV>(Registry->ConstructNewObjectVariable(UGorgeousInsightBlueprintStats_OV::StaticClass()));
		if (StatsRegistry)
		{
			// Make it a global object variable
			Registry->RegisterGlobalObjectVariable(StatsRegistry);
		}
	}

	if (StatsRegistry)
	{
		// Ensure the system map entry exists
		if (!StatsRegistry->SystemStatsMap.Contains(SystemName))
		{
			StatsRegistry->SystemStatsMap.Add(SystemName, FGorgeousBlueprintSystemStatsData());
			StatsRegistry->MarkDirty();
		}
		
		return StatsRegistry->GetObjectVariableIdentifier();
	}

	return FGuid();
}

void UGorgeousInsightBlueprintStats_OV::RegisterBlueprintSystemAction(UObject* WorldContextObject, const FGuid& RegistryGuid, FName SystemName, FName ActionName, FGameplayTag SignalBridgeTag)
{
	if (!RegistryGuid.IsValid() || !WorldContextObject)
	{
		return;
	}

	UGorgeousObjectVariableRegistry_GIS* Registry = UGorgeousObjectVariableRegistry_GIS::Get(WorldContextObject);
	if (Registry)
	{
		if (UGorgeousObjectVariable* OV = Registry->GetObjectVariableByIdentifier(RegistryGuid))
		{
			if (UGorgeousInsightBlueprintStats_OV* StatsRegistry = Cast<UGorgeousInsightBlueprintStats_OV>(OV))
			{
				FGorgeousBlueprintSystemStatsData& Data = StatsRegistry->SystemStatsMap.FindOrAdd(SystemName);
				
				// Check if action already exists
				bool bExists = false;
				for (const FGorgeousBlueprintInsightAction& Action : Data.Actions)
				{
					if (Action.ActionName == ActionName)
					{
						bExists = true;
						break;
					}
				}

				if (!bExists)
				{
					FGorgeousBlueprintInsightAction NewAction;
					NewAction.ActionName = ActionName;
					NewAction.SignalBridgeTag = SignalBridgeTag;
					Data.Actions.Add(NewAction);
					StatsRegistry->MarkDirty();
				}
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

	UGorgeousObjectVariableRegistry_GIS* Registry = UGorgeousObjectVariableRegistry_GIS::Get(WorldContextObject);
	if (Registry)
	{
		if (UGorgeousObjectVariable* OV = Registry->GetObjectVariableByIdentifier(RegistryGuid))
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
							StatsRegistry->MarkDirty();
							break;
						}
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

	UGorgeousObjectVariableRegistry_GIS* Registry = UGorgeousObjectVariableRegistry_GIS::Get(WorldContextObject);
	if (Registry)
	{
		if (UGorgeousObjectVariable* OV = Registry->GetObjectVariableByIdentifier(RegistryGuid))
		{
			if (UGorgeousInsightBlueprintStats_OV* StatsRegistry = Cast<UGorgeousInsightBlueprintStats_OV>(OV))
			{
				if (StatsRegistry->SystemStatsMap.Remove(SystemName) > 0)
				{
					StatsRegistry->MarkDirty();
				}
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
		UGorgeousObjectVariableRegistry_GIS* Registry = UGorgeousObjectVariableRegistry_GIS::Get(Stack.Object);
		if (Registry)
		{
			if (UGorgeousObjectVariable* OV = Registry->GetObjectVariableByIdentifier(RegistryGuid))
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
						StatsRegistry->MarkDirty();
					}
				}
			}
		}
	}

	P_NATIVE_END;
}
