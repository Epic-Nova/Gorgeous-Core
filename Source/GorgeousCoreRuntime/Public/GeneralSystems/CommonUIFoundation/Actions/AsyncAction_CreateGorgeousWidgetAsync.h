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
#include "Kismet/BlueprintAsyncActionBase.h"
#include "UObject/SoftObjectPtr.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "AsyncAction_CreateGorgeousWidgetAsync.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UUserWidget;
class UCommonActivatableWidget;
//<------------------------------------------------------------->

// Broadcasts the widget created by the asynchronous action.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGorgeousWidgetAsyncDelegate, UUserWidget*, Widget);

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Async Action Create Gorgeous Widget Async
| Functional Name: UAsyncAction_CreateGorgeousWidgetAsync
| Parent Class: UBlueprintAsyncActionBase
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Async node to create a widget without frame hitches. Automatically applies
| the current global theme upon creation.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Actions/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Actions/AsyncAction_CreateGorgeousWidgetAsync",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Actions/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UAsyncAction_CreateGorgeousWidgetAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	//<============================--- Overrides ---============================>
	#pragma region Overrides
public:
	// Starts loading the requested widget class.
	virtual void Activate() override;
	//<------------------------------------------------------------------------->
	#pragma endregion Overrides


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:
	/**
	 * Asynchronously loads and creates a Gorgeous widget.
	 *
	 * @param InWorldContextObject The world context that owns the widget.
	 * @param WidgetClass The widget class to load and create.
	 * @param bSuspendInputUntilComplete Whether input remains suspended during loading.
	 * @return The asynchronous action that reports widget creation.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gorgeous Core|Common UI Foundation|Extensions", meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"))
	static UAsyncAction_CreateGorgeousWidgetAsync* CreateGorgeousWidgetAsync(
		UObject* InWorldContextObject,
		TSoftClassPtr<UUserWidget> WidgetClass,
		bool bSuspendInputUntilComplete = true
	);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:
	// Fired when the widget has been successfully loaded and created.
	UPROPERTY(BlueprintAssignable)
	FGorgeousWidgetAsyncDelegate OnComplete;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables


	//<============================--- C++ Only ---=============================>
	#pragma region C++ Only
private:
	// Creates the widget after its class has loaded.
	void OnWidgetClassLoaded();
	//<------------------------------------------------------------------------->
	#pragma endregion C++ Only


	//<============================--- Variables ---============================>
	#pragma region Variables
private:
	// Stores the world context that owns the asynchronous operation.
	UPROPERTY(Transient)
	TObjectPtr<UObject> WorldContextObject;

	// Stores the widget class to load.
	UPROPERTY(Transient)
	TSoftClassPtr<UUserWidget> WidgetSoftClass;

	// Records whether input should remain suspended while loading.
	UPROPERTY(Transient)
	bool bSuspendInput = false;

	// Identifies the temporary input suspension.
	UPROPERTY(Transient)
	FName SuspendToken;
	//<------------------------------------------------------------------------->
	#pragma endregion Variables
};