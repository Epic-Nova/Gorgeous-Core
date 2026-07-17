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
#include "Blueprint/DragDropOperation.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousDragDropFoundation.generated.h"
//<-------------------------------------------------------------------------->

/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Drag Drop Foundation
| Functional Name: UGorgeousDragDropFoundation
| Parent Class: UDragDropOperation
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Universal base class for stateful drag and drop interactions in the
| Gorgeous ecosystem. Supports both standard PC (mouse) and state-based
| Gamepad flows.
<--------------------------------------------------------------------------->
<==========================================================================>
*/
UCLASS(Abstract, BlueprintType, Blueprintable,
	meta = (
		DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Interaction/Overview",
		DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Interaction/GorgeousDragDropFoundation",
		DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Interaction/Examples/"
		)
)
class GORGEOUSCORERUNTIME_API UGorgeousDragDropFoundation : public UDragDropOperation
{
	GENERATED_BODY()

	//<=======================--- Blueprint Functions ---=======================>
	#pragma region Blueprint Functions
public:
	// Starts the drag operation.
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous|Interaction")
	void OnDragStarted();

	/**
	 * Handles a highlighted potential drop target.
	 *
	 * @param Target The highlighted drop target.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous|Interaction")
	void OnTargetHighlighted(UObject* Target);

	/**
	 * Handles a confirmed drag target.
	 *
	 * @param Target The target that receives the drop.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous|Interaction")
	void OnDragConfirmed(UObject* Target);

	/**
	 * Resumes an interaction that is waiting for a target.
	 *
	 * @param Target The target selected to resume the interaction.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Interaction")
	void ResumeStatefulInteraction(UObject* Target);
	//<------------------------------------------------------------------------->
	#pragma endregion Blueprint Functions


	//<====================--- UAT/UBT Exposed Variables ---====================>
	#pragma region UAT/UBT Exposed Variables
public:
	// Stores the context in which this drag operation runs.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous|Interaction")
	FGameplayTag DragContext;
	// Stores the item or action carried by the drag operation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous|Interaction")
	FGameplayTag PayloadTag;
	//<------------------------------------------------------------------------->
	#pragma endregion UAT/UBT Exposed Variables
};