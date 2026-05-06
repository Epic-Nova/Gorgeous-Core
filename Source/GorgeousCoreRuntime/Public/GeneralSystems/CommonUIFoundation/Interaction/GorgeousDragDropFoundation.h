// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "GameplayTagContainer.h"
#include "GorgeousDragDropFoundation.generated.h"

/**
 * Universal base class for stateful drag and drop interactions in the Gorgeous ecosystem.
 * Supports both standard PC (mouse) and state-based Gamepad flows.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class GORGEOUSCORERUNTIME_API UGorgeousDragDropFoundation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	/** The gameplay tag representing the context of this drag operation (e.g., UI.DragContext.Inventory). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous|Interaction")
	FGameplayTag DragContext;

	/** The tag of the item or action being dragged. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gorgeous|Interaction")
	FGameplayTag PayloadTag;

	/** Called when the drag operation starts. */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous|Interaction")
	void OnDragStarted();

	/** Called when a potential drop target is highlighted (e.g., hovered or selected via gamepad). */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous|Interaction")
	void OnTargetHighlighted(UObject* Target);

	/** Called when the drag is confirmed (dropped). */
	UFUNCTION(BlueprintNativeEvent, Category = "Gorgeous|Interaction")
	void OnDragConfirmed(UObject* Target);

	/** 
	 * For Gamepads: Resumes a stateful interaction that was waiting for a target.
	 */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous|Interaction")
	void ResumeStatefulInteraction(UObject* Target);
};
