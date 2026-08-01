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
//<--------------------------=== Module Includes ===------------------------->
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GorgeousUIFoundationStructures.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousUIFoundationLibrary.generated.h"
//<-------------------------------------------------------------------------->

//<=================--- Forward Declarations ---=================>
class UGorgeousPrimaryGameLayout;
//<------------------------------------------------------------->
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous UIFoundation Library
| Functional Name: UGorgeousUIFoundationLibrary
| Parent Class: UBlueprintFunctionLibrary
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Blueprint Function Library for the Gorgeous Core|Common UI Foundation
| Foundation system.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/GorgeousUIFoundationLibrary",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousUIFoundationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:
	/**
	 * Resolves the primary Gorgeous layout for a world context.
	 *
	 * @param WorldContextObject The object that supplies the target world.
	 * @return The primary layout, or null when it is unavailable.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation", meta = (WorldContext = "WorldContextObject"))
	static UGorgeousPrimaryGameLayout* GetGorgeousPrimaryLayout(const UObject* WorldContextObject);

	/**
	 * Resolves the primary Gorgeous layout for a player controller.
	 *
	 * @param PlayerController The controller that owns the target local player.
	 * @return The primary layout, or null when it is unavailable.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation")
	static UGorgeousPrimaryGameLayout* GetGorgeousPrimaryLayoutFromController(APlayerController* PlayerController);

	/**
	 * Finds and removes a widget from its layout layer.
	 *
	 * @param ActivatableWidget The widget to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation")
	static void FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);

	/**
	 * Resolves the widget container registered for a layer.
	 *
	 * @param WorldContextObject The object that supplies the target world.
	 * @param LayerTag The tag identifying the requested layer.
	 * @return The matching layer container, or null when it is unavailable.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation", meta = (WorldContext = "WorldContextObject"))
	static UCommonActivatableWidgetContainerBase* GetLayerWidget(const UObject* WorldContextObject, FGameplayTag LayerTag);

	/**
	 * Creates a new UI update payload.
	 *
	 * @return A new empty UI update payload.
	 */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation|Payload")
	static FGorgeousUIUpdatePayload MakeGorgeousUIUpdatePayload();

	/**
	 * Adds a text value to a UI update payload.
	 *
	 * @param Payload The payload to update.
	 * @param PropertyName The property that receives the value.
	 * @param Value The text value to assign.
	 * @return The updated payload.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Payload")
	static UPARAM(DisplayName = "Payload") FGorgeousUIUpdatePayload& AddTextToUIUpdate(UPARAM(ref) FGorgeousUIUpdatePayload& Payload, FName PropertyName, FText Value);

	/**
	 * Adds a color value to a UI update payload.
	 *
	 * @param Payload The payload to update.
	 * @param PropertyName The property that receives the value.
	 * @param Value The color value to assign.
	 * @return The updated payload.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Payload")
	static UPARAM(DisplayName = "Payload") FGorgeousUIUpdatePayload& AddColorToUIUpdate(UPARAM(ref) FGorgeousUIUpdatePayload& Payload, FName PropertyName, FLinearColor Value);

	/**
	 * Adds a float value to a UI update payload.
	 *
	 * @param Payload The payload to update.
	 * @param PropertyName The property that receives the value.
	 * @param Value The float value to assign.
	 * @return The updated payload.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Payload")
	static UPARAM(DisplayName = "Payload") FGorgeousUIUpdatePayload& AddFloatToUIUpdate(UPARAM(ref) FGorgeousUIUpdatePayload& Payload, FName PropertyName, float Value);

	/**
	 * Adds an integer value to a UI update payload.
	 *
	 * @param Payload The payload to update.
	 * @param PropertyName The property that receives the value.
	 * @param Value The integer value to assign.
	 * @return The updated payload.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Payload")
	static UPARAM(DisplayName = "Payload") FGorgeousUIUpdatePayload& AddIntToUIUpdate(UPARAM(ref) FGorgeousUIUpdatePayload& Payload, FName PropertyName, int32 Value);

	/**
	 * Adds an object reference to a UI update payload.
	 *
	 * @param Payload The payload to update.
	 * @param PropertyName The property that receives the value.
	 * @param Value The object reference to assign.
	 * @return The updated payload.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Payload")
	static UPARAM(DisplayName = "Payload") FGorgeousUIUpdatePayload& AddObjectToUIUpdate(UPARAM(ref) FGorgeousUIUpdatePayload& Payload, FName PropertyName, UObject* Value);

	/**
	 * Adds a boolean value to a UI update payload.
	 *
	 * @param Payload The payload to update.
	 * @param PropertyName The property that receives the value.
	 * @param Value The Boolean value to assign.
	 * @return The updated payload.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Payload")
	static UPARAM(DisplayName = "Payload") FGorgeousUIUpdatePayload& AddBoolToUIUpdate(UPARAM(ref) FGorgeousUIUpdatePayload& Payload, FName PropertyName, bool Value);

	/**
	 * Adds a padding/margin value to a UI update payload.
	 *
	 * @param Payload The payload to update.
	 * @param PropertyName The property that receives the value.
	 * @param Value The margin value to assign.
	 * @return The updated payload.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Payload")
	static UPARAM(DisplayName = "Payload") FGorgeousUIUpdatePayload& AddPaddingToUIUpdate(UPARAM(ref) FGorgeousUIUpdatePayload& Payload, FName PropertyName, FMargin Value);

	/**
	 * Adds a grid update payload to a UI update payload.
	 *
	 * @param Payload The payload to update.
	 * @param PropertyName The property that receives the value.
	 * @param Value The grid payload to assign.
	 * @return The updated payload.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Payload")
	static UPARAM(DisplayName = "Payload") FGorgeousUIUpdatePayload& AddGridToUIUpdate(UPARAM(ref) FGorgeousUIUpdatePayload& Payload, FName PropertyName, FGorgeousGridUpdatePayload Value);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions
};