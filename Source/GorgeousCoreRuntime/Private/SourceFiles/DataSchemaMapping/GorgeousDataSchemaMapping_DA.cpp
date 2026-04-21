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
#include "DataSchemaMapping/GorgeousDataSchemaMapping_DA.h"

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Helpers/GorgeousPropertyPathHelper.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Engine/DataTable.h"
#include "Misc/PackageName.h"
#include "UObject/UnrealType.h"
//<-------------------------------------------------------------------------->

namespace
{
	constexpr int32 MaxPropertyTraversalDepth = 4;

	void BuildPropertyPathVariants(const FString& Prefix, const FProperty* Property, TArray<FString>& OutPathVariants)
	{
		OutPathVariants.Reset();
		if (!Property)
		{
			return;
		}

		const FString InternalSegment = Property->GetName();
		const FString FriendlySegment = GorgeousPropertyPath::GetPreferredPropertySegment(Property);

		auto BuildPath = [&Prefix](const FString& Segment)
		{
			return Prefix.IsEmpty()
				? Segment
				: FString::Printf(TEXT("%s.%s"), *Prefix, *Segment);
		};

		if (!FriendlySegment.IsEmpty())
		{
			OutPathVariants.Add(BuildPath(FriendlySegment));
		}

		if (!InternalSegment.IsEmpty() && !InternalSegment.Equals(FriendlySegment, ESearchCase::IgnoreCase))
		{
			OutPathVariants.Add(BuildPath(InternalSegment));
		}
	}

	void CollectStructPropertyPaths(const UStruct* InStructType, const FString& Prefix, int32 Depth, TSet<const UStruct*>& VisitedStructs, TArray<FString>& OutPaths)
	{
		if (!InStructType || Depth > MaxPropertyTraversalDepth)
		{
			return;
		}

		if (VisitedStructs.Contains(InStructType))
		{
			return;
		}

		VisitedStructs.Add(InStructType);

		for (TFieldIterator<FProperty> PropertyIt(InStructType, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
		{
			const FProperty* Property = *PropertyIt;
			if (!Property || Property->HasAnyPropertyFlags(CPF_Transient))
			{
				continue;
			}

			TArray<FString> PathVariants;
			BuildPropertyPathVariants(Prefix, Property, PathVariants);
			for (const FString& Path : PathVariants)
			{
				OutPaths.AddUnique(Path);

				if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
				{
					CollectStructPropertyPaths(StructProperty->Struct, Path, Depth + 1, VisitedStructs, OutPaths);
				}
			}
		}

		VisitedStructs.Remove(InStructType);
	}

	const UStruct* ResolveSourceRootStruct(const UGorgeousDataSchemaMapping_DA* SchemaMap)
	{
		if (!SchemaMap)
		{
			return nullptr;
		}

		switch (SchemaMap->SourceDefinition.SourceKind)
		{
		case EGorgeousDataSchemaSourceKind_E::Source_DataTableRows:
			if (UScriptStruct* RequiredRowStruct = SchemaMap->SourceDefinition.RequiredDataTableRowStruct.LoadSynchronous())
			{
				return RequiredRowStruct;
			}

			if (UDataTable* SourceDataTable = SchemaMap->SourceDefinition.SourceDataTable.LoadSynchronous())
			{
				return SourceDataTable->GetRowStruct();
			}

			return nullptr;

		case EGorgeousDataSchemaSourceKind_E::Source_DataAsset:
			return SchemaMap->SourceDefinition.RequiredSourceDataAssetClass.LoadSynchronous();

		case EGorgeousDataSchemaSourceKind_E::Source_BlueprintClassDefaults:
		case EGorgeousDataSchemaSourceKind_E::Source_AnyUObject:
		default:
			return SchemaMap->SourceDefinition.RequiredSourceClass.LoadSynchronous();
		}
	}

	const UStruct* ResolveTargetRootStruct(const UGorgeousDataSchemaMapping_DA* SchemaMap)
	{
		if (!SchemaMap)
		{
			return nullptr;
		}

		return SchemaMap->TargetDefinition.TargetClass.LoadSynchronous();
	}
}

bool UGorgeousDataSchemaMapping_DA::IsMigrationActionEnabled() const
{
	return MigrationDefinition.bEnableContentBrowserMigrationAction;
}

FString UGorgeousDataSchemaMapping_DA::GetNormalizedTargetContentRootPath() const
{
	FString NormalizedPath = TargetDefinition.TargetContentRootPath.TrimStartAndEnd();
	if (NormalizedPath.IsEmpty())
	{
		return FString();
	}

	if (NormalizedPath.StartsWith(TEXT("Game/")))
	{
		NormalizedPath = FString::Printf(TEXT("/%s"), *NormalizedPath);
	}
	else if (!NormalizedPath.StartsWith(TEXT("/")))
	{
		NormalizedPath = FString::Printf(TEXT("/Game/%s"), *NormalizedPath);
	}

	if (!NormalizedPath.StartsWith(TEXT("/Game")))
	{
		return FString();
	}

	while (NormalizedPath.EndsWith(TEXT("/")))
	{
		NormalizedPath.LeftChopInline(1);
	}

	if (!FPackageName::IsValidLongPackageName(NormalizedPath))
	{
		return FString();
	}

	return NormalizedPath;
}

FText UGorgeousDataSchemaMapping_DA::GetResolvedMigrationActionLabel() const
{
	if (!MigrationDefinition.MenuActionLabel.IsEmpty())
	{
		return MigrationDefinition.MenuActionLabel;
	}

	return FText::FromString(GetName());
}

bool UGorgeousDataSchemaMapping_DA::IsDirectDataTableExecutionEnabled() const
{
	return SourceDefinition.SourceKind == EGorgeousDataSchemaSourceKind_E::Source_DataTableRows
		&& !SourceDefinition.SourceDataTable.IsNull();
}

TArray<FString> UGorgeousDataSchemaMapping_DA::GetSourceFieldPathOptions() const
{
	TArray<FString> SourcePaths;
	if (const UStruct* SourceRootStruct = ResolveSourceRootStruct(this))
	{
		TSet<const UStruct*> VisitedStructs;
		CollectStructPropertyPaths(SourceRootStruct, FString(), 0, VisitedStructs, SourcePaths);
		SourcePaths.Sort();
	}

	return SourcePaths;
}

TArray<FString> UGorgeousDataSchemaMapping_DA::GetTargetFieldPathOptions() const
{
	TArray<FString> TargetPaths;
	if (const UStruct* TargetRootStruct = ResolveTargetRootStruct(this))
	{
		TSet<const UStruct*> VisitedStructs;
		CollectStructPropertyPaths(TargetRootStruct, FString(), 0, VisitedStructs, TargetPaths);
		TargetPaths.Sort();
	}

	return TargetPaths;
}
