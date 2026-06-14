// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|              Gorgeous Events - Events functionality provider              |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Engine Includes ===------------------------->
#include "EditorValidatorBase.h"
#include "Containers/Ticker.h"
//----------------=== Third Party & Miscellaneous Includes ===--------------->
#include "GorgeousGeneralSystemValidator.generated.h"
//<-------------------------------------------------------------------------->

/**
 * Validator for DataRegistry assets used in the Gorgeous Playlist System.
 */
UCLASS()
class UGorgeousGeneralSystemValidator : public UEditorValidatorBase
{
	GENERATED_BODY()

	// Constructor
	UGorgeousGeneralSystemValidator();
	
	// Destructor
	~UGorgeousGeneralSystemValidator();
	
	//<============================--- Overrides ---============================>
public:

	// Override of CanValidateAsset to specify that this validator only applies to general system primary data assets under 'Systems/'.
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	
	// Override of ValidateLoadedAsset to perform the actual validation logic for general systems.
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
	
	// Cleanup for asset creation bindings
	virtual void BeginDestroy() override;
	//<------------------------------------------------------------------------->
	
	//<=======================--- Blueprint Functions ---=======================>
	
	/// Hyperlink action handler — registers general systems primary data asset
	UFUNCTION()
	void HandleRegisterAssetRegistryEntry(const FString& Payload);
	//<------------------------------------------------------------------------->
	
	
	//<============================--- C++ Only ---=============================>
	
	UFUNCTION()
	void HandleRegisterDataRegistryEntry(const FString& Payload);

	UFUNCTION()
	bool HandleCanRegisterDataRegistryEntry(const FString& Payload);

	void DiscoverAndRegisterGorgeousPluginSystems();
	void ScanAndRecreateMissingPDAs();
	
	static void QueueAssetsForAsyncValidation(const TArray<FAssetData>& Assets);
	static void RequestSystemValidationScan();
	static TArray<FString> GetGorgeousSystemDirectories();
	
	bool IsSystemAssetManagerRegistered() const;
	bool IsSystemComponent(const FString Name, const uint8 CheckMode = 0) const;
	
	UClass* FindSystemDefaultComponent(const FString& PDA_SystemRoot, const bool bIsManager);
		
	
	void ValidatePDA(class UGeneralSystemConfiguration_PDA* PDA, FDataValidationContext& Context);
	
	void HandleAssetAdded(const FAssetData& AssetData);
	void OnSafeProcessAsset(UWorld* World, FAssetData AssetData);
	
	static FString GetSystemRootPath(const FString& Path);
	static FString ConstructPDASystemName(const UClass* InComponentClass, FString& OutSystemName);
	
	static UGeneralSystemConfiguration_PDA* CreatePDA(const UClass* ComponentClass);
    
	static UGeneralSystemConfiguration_PDA* FindExistingPDA(const UClass* ComponentClass);
	//<------------------------------------------------------------------------->
	
	//<============================--- Variables ---============================>

	// A list of already processed assets
	static TSet<FName> ProcessedAssets;
	
	// A list of created system PDAs
	static TSet<FString> CreatedSystems;
	
	// Assets that are pending to be saved
	TArray<TSoftObjectPtr<UGeneralSystemConfiguration_PDA>> PendingSaveAssets;
	
	FTimerHandle ScanTimerHandle;

	// Ticker handle used to wait for other Gorgeous plugins to register so
	// we can ensure their /Systems paths are added to AssetManager scanning.
	FTSTicker::FDelegateHandle SystemPathsTickerHandle;

	// Set true after we successfully discovered/registered plugin system paths once.
	static bool bPluginSystemsDiscovered;
	static bool bSystemValidationScanStarted;

	// Ticker callback used to wait for other plugins to register
	bool TickUpdateSystemPaths(float DeltaTime);

	// Ticker used for background validation processing
	static FTSTicker::FDelegateHandle AsyncValidationTickerHandle;
	static TArray<FAssetData> AsyncValidationQueue;
	static TSharedPtr<class SNotificationItem> AsyncProgressNotification;
	static int32 TotalAsyncAssets;

	bool TickAsyncValidation(float DeltaTime);

	//<------------------------------------------------------------------------->

};
