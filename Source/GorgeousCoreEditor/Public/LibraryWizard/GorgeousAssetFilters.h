// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "Filters/FilterBase.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LibraryWizard/GorgeousLibraryTypes.h"

/**
 * A standard custom filter for the Gorgeous Library that checks asset metadata tags.
 * Implements all FFilterBase pure virtuals so SFilterBar can display and manage it.
 */
class FGorgeousAssetFilter : public FFilterBase<TSharedPtr<FAssetData>>
{
public:
	FGorgeousAssetFilter(const FGorgeousLibraryFilter& InDescriptor)
		: FFilterBase<TSharedPtr<FAssetData>>(nullptr)
		, Descriptor(InDescriptor)
	{
	}

	FGorgeousAssetFilter(const FGorgeousLibraryFilter& InDescriptor, TSharedPtr<FFilterCategory> InCategory)
		: FFilterBase<TSharedPtr<FAssetData>>(MoveTemp(InCategory))
		, Descriptor(InDescriptor)
	{
	}

	//<=====--- FFilterBase pure virtuals ---=====>

	virtual FString GetName() const override
	{
		return Descriptor.FilterId.ToString();
	}

	virtual FText GetDisplayName() const override
	{
		return Descriptor.DisplayName;
	}

	virtual FText GetToolTipText() const override
	{
		return Descriptor.DisplayName;
	}

	virtual FLinearColor GetColor() const override
	{
		return Descriptor.Color;
	}

	virtual FName GetIconName() const override
	{
		return Descriptor.IconName;
	}

	virtual bool IsInverseFilter() const override
	{
		return false;
	}

	virtual void ActiveStateChanged(bool bActive) override
	{
		// No special per-filter state needed
	}

	virtual void ModifyContextMenu(FMenuBuilder& MenuBuilder) override
	{
		// No extra context menu entries
	}

	virtual void SaveSettings(const FString& IniFilename, const FString& IniSection, const FString& SettingsString) const override
	{
		// No persistent settings
	}

	virtual void LoadSettings(const FString& IniFilename, const FString& IniSection, const FString& SettingsString) override
	{
		// No persistent settings
	}

	//<------------------------------------------------>

	//<=====--- IFilter pure virtual ---=====>

	virtual bool PassesFilter(TSharedPtr<FAssetData> InItem) const override
	{
		if (!InItem.IsValid())
		{
			return false;
		}

		// Check if the metadata tag (e.g. ItemType, Rarity) contains our FilterId.
		// Participants are expected to expose these tags via UPROPERTY metadata.
		FString TagValue;
		if (InItem->GetTagValue(Descriptor.FilterCategory, TagValue))
		{
			// Handle comma-separated values (e.g. multi-type items)
			TArray<FString> Values;
			TagValue.ParseIntoArray(Values, TEXT(","), true);
			for (const FString& Value : Values)
			{
				if (Value.TrimStartAndEnd().Equals(Descriptor.FilterId.ToString(), ESearchCase::IgnoreCase))
				{
					return true;
				}
			}
		}

		return false;
	}

	//<---------------------------------------->

	/** The descriptor from the participant that defines this filter's display/logic. */
	FGorgeousLibraryFilter Descriptor;
};
