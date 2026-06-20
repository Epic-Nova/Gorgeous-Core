// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "LibraryWizard/IGorgeousLibraryParticipant.h"
#include "LibraryWizard/GorgeousInstallableSystem_DA.h"

class GORGEOUSCOREEDITOR_API FGorgeousCoreLibraryParticipant final : public IGorgeousLibraryParticipant
{
public:
	FGorgeousCoreLibraryParticipant();
	virtual ~FGorgeousCoreLibraryParticipant();

	// IGorgeousLibraryParticipant interface
	virtual FName GetParticipantName() const override;
	virtual FText GetParticipantDisplayName() const override;
	virtual FName GetParticipantIcon() const override;
	virtual TArray<FGorgeousLibraryCategoryDescriptor> GetLibraryCategories() const override;
	virtual void OnAssetDoubleClicked(const FAssetData& Asset, const FGorgeousLibraryCategoryDescriptor& Category) override;
	virtual void OnBuildContextMenu(class FMenuBuilder& MenuBuilder, const TArray<FAssetData>& SelectedAssets, const FGorgeousLibraryCategoryDescriptor& Category) override;
	virtual TSharedPtr<FSlateBrush> GetAssetIcon(const FAssetData& Asset) override;
	virtual FText GetAssetDescription(const FAssetData& Asset) override;
	virtual void OnCategoryActionExecuted(FName CategoryId, FName ActionId) override;

	/** Refresh the transient offline cache data assets */
	void RefreshOfflineCache();

private:
	/** Keep references to transient assets so they aren't garbage collected */
	TArray<UObject*> TransientAssets;
};
