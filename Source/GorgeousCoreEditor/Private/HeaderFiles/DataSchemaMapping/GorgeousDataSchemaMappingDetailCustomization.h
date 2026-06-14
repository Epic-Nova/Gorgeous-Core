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
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "IDetailCustomization.h"
#include "Input/Reply.h"
#include "Layout/Visibility.h"
#include "Types/SlateEnums.h"
#include "UObject/WeakObjectPtr.h"
//<-------------------------------------------------------------------------->

//<===========--- Forward Declarations ---===========>
struct FAssetData;
class IDetailLayoutBuilder;
class IPropertyHandle;
class SButton;
class UGorgeousDataSchemaMapping_DA;
//<-------------------------------------------------->

class FGorgeousDataSchemaMappingDetailCustomization final : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	FReply HandleBrowseTargetPathClicked();
	void HandlePathPicked(const FString& PickedPath);
	EVisibility GetSourceDefinitionPropertyVisibility(FName SourcePropertyName) const;
	bool IsSourceDataTablePickerEnabled() const;
	EVisibility GetSourceDataTablePickerVisibility() const;
	bool ShouldFilterSourceDataTableAsset(const FAssetData& AssetData) const;

private:
	TSharedPtr<IPropertyHandle> TargetPathHandle;
	TSharedPtr<IPropertyHandle> SourceKindHandle;
	TSharedPtr<IPropertyHandle> RequiredDataTableRowStructHandle;
	TSharedPtr<IPropertyHandle> SourceDataTableHandle;
	TWeakObjectPtr<UGorgeousDataSchemaMapping_DA> EditedSchemaAsset;
	TWeakPtr<SButton> BrowseButton;
};
