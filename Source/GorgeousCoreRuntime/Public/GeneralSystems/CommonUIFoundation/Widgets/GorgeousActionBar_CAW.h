// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GeneralSystems/CommonUIFoundation/Widgets/GorgeousActivatableWidget.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationStructures.h"
#include "GorgeousActionBar_CAW.generated.h"

/**
 * Base widget for the HUD Action Bar stripe.
 * Displays available input actions, their icons, and names.
 * 
 * @TODO
 */
UCLASS(Abstract, Blueprintable)
class GORGEOUSCORERUNTIME_API UGorgeousActionBar_CAW : public UGorgeousActivatableWidget
{
	GENERATED_BODY()

public:
	UGorgeousActionBar_CAW(const FObjectInitializer& ObjectInitializer);

public:
	/** 
	 * Updates the action bar with new entries. 
	 * Blueprint should implement this to rebuild the UI (e.g. clear box and add children).
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gorgeous|UI")
	void UpdateActionBar(const TArray<FGorgeousActionBarEntry_S>& Entries);

protected:
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
};


/**
 * Maybe replace with the common ui action bar as this look really nice
* JournalHandle = RegisterUIActionBinding(FBindUIActionArgs(ToggleJournalAction, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleToggleJournal)));
void UPlayerMenusBase::NativeOnInitialized()
{
Super::NativeOnInitialized();

FBindUIActionArgs InventoryBindArgs = FBindUIActionArgs(ToggleInventoryAction, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleToggleInventory));
InventoryBindArgs.bIsPersistent = true;
InventoryBindArgs.InputMode = ECommonInputMode::All;

InventoryHandle = RegisterUIActionBinding(InventoryBindArgs);
JournalHandle = RegisterUIActionBinding(FBindUIActionArgs(ToggleJournalAction, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleToggleJournal)));
CraftingHandle = RegisterUIActionBinding(FBindUIActionArgs(ToggleCraftingAction, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleToggleCrafting)));
QuickTransferHandle = RegisterUIActionBinding(FBindUIActionArgs(QuickTransferAction, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleQuickTransfer)));
//RemoveActionBinding(CraftingHandle);
RemoveActionBinding(InventoryHandle);
RemoveActionBinding(QuickTransferHandle);
RemoveActionBinding(JournalHandle);

// Used to disable native TAB behavior
TSharedRef<FNavigationConfig> NavigationConfig = MakeShared<FNavigationConfig>();
NavigationConfig->bTabNavigation = false; //prevent a windows bug

FSlateApplication::Get().SetNavigationConfig(NavigationConfig);

}
if (EntryIndex == 1)
{
if (GetActionBindings().Contains(QuickTransferHandle)) return;

AddActionBinding(QuickTransferHandle);
}
else
{
RemoveActionBinding(QuickTransferHandle);
}
**/