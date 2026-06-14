// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GorgeousUIFoundationStructures.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "GorgeousUIFoundationLibrary.generated.h"

class UGorgeousPrimaryGameLayout;
/**
<<<<<<< HEAD
 * Blueprint Function Library for the Gorgeous Core|Common UI Foundation Foundation system.
=======
 * Blueprint Function Library for the Gorgeous UI Foundation system.
>>>>>>> b4c134c (Some other changes i dont remember)
 */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousUIFoundationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns the gorgeous primary game layout for the specified world context object. */
<<<<<<< HEAD
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation", meta = (WorldContext = "WorldContextObject"))
	static UGorgeousPrimaryGameLayout* GetGorgeousPrimaryLayout(const UObject* WorldContextObject);

	/** Returns the gorgeous primary game layout for the specified player controller. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation")
	static UGorgeousPrimaryGameLayout* GetGorgeousPrimaryLayoutFromController(APlayerController* PlayerController);

	/** Finds a widget on any layer and removes it. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation")
	static void FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);

	/** Returns the widget container for a specific layer tag. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation", meta = (WorldContext = "WorldContextObject"))
=======
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI", meta = (WorldContext = "WorldContextObject"))
	static UGorgeousPrimaryGameLayout* GetGorgeousPrimaryLayout(const UObject* WorldContextObject);

	/** Returns the gorgeous primary game layout for the specified player controller. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI")
	static UGorgeousPrimaryGameLayout* GetGorgeousPrimaryLayoutFromController(APlayerController* PlayerController);

	/** Finds a widget on any layer and removes it. */
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI")
	static void FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);

	/** Returns the widget container for a specific layer tag. */
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI", meta = (WorldContext = "WorldContextObject"))
>>>>>>> b4c134c (Some other changes i dont remember)
	static UCommonActivatableWidgetContainerBase* GetLayerWidget(const UObject* WorldContextObject, FGameplayTag LayerTag);

	/**
	 * Creates a new UI update payload.
	 */
<<<<<<< HEAD
	UFUNCTION(BlueprintPure, Category = "Gorgeous Core|Common UI Foundation|Payload")
=======
	UFUNCTION(BlueprintPure, Category = "Gorgeous UI|Payload")
>>>>>>> b4c134c (Some other changes i dont remember)
	static FGorgeousUIUpdatePayload MakeGorgeousUIUpdatePayload();

	/**
	 * Adds a text value to a UI update payload.
	 */
<<<<<<< HEAD
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Payload")
=======
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Payload")
>>>>>>> b4c134c (Some other changes i dont remember)
	static UPARAM(DisplayName = "Payload") FGorgeousUIUpdatePayload& AddTextToUIUpdate(UPARAM(ref) FGorgeousUIUpdatePayload& Payload, FName PropertyName, FText Value);

	/**
	 * Adds a color value to a UI update payload.
	 */
<<<<<<< HEAD
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Payload")
=======
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Payload")
>>>>>>> b4c134c (Some other changes i dont remember)
	static UPARAM(DisplayName = "Payload") FGorgeousUIUpdatePayload& AddColorToUIUpdate(UPARAM(ref) FGorgeousUIUpdatePayload& Payload, FName PropertyName, FLinearColor Value);

	/**
	 * Adds a float value to a UI update payload.
	 */
<<<<<<< HEAD
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Payload")
=======
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Payload")
>>>>>>> b4c134c (Some other changes i dont remember)
	static UPARAM(DisplayName = "Payload") FGorgeousUIUpdatePayload& AddFloatToUIUpdate(UPARAM(ref) FGorgeousUIUpdatePayload& Payload, FName PropertyName, float Value);

	/**
	 * Adds an integer value to a UI update payload.
	 */
<<<<<<< HEAD
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Payload")
=======
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Payload")
>>>>>>> b4c134c (Some other changes i dont remember)
	static UPARAM(DisplayName = "Payload") FGorgeousUIUpdatePayload& AddIntToUIUpdate(UPARAM(ref) FGorgeousUIUpdatePayload& Payload, FName PropertyName, int32 Value);

	/**
	 * Adds an object reference to a UI update payload.
	 */
<<<<<<< HEAD
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Payload")
=======
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Payload")
>>>>>>> b4c134c (Some other changes i dont remember)
	static UPARAM(DisplayName = "Payload") FGorgeousUIUpdatePayload& AddObjectToUIUpdate(UPARAM(ref) FGorgeousUIUpdatePayload& Payload, FName PropertyName, UObject* Value);

	/**
	 * Adds a boolean value to a UI update payload.
	 */
<<<<<<< HEAD
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Payload")
=======
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Payload")
>>>>>>> b4c134c (Some other changes i dont remember)
	static UPARAM(DisplayName = "Payload") FGorgeousUIUpdatePayload& AddBoolToUIUpdate(UPARAM(ref) FGorgeousUIUpdatePayload& Payload, FName PropertyName, bool Value);

	/**
	 * Adds a padding/margin value to a UI update payload.
	 */
<<<<<<< HEAD
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Payload")
=======
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Payload")
>>>>>>> b4c134c (Some other changes i dont remember)
	static UPARAM(DisplayName = "Payload") FGorgeousUIUpdatePayload& AddPaddingToUIUpdate(UPARAM(ref) FGorgeousUIUpdatePayload& Payload, FName PropertyName, FMargin Value);

	/**
	 * Adds a grid update payload to a UI update payload.
	 */
<<<<<<< HEAD
	UFUNCTION(BlueprintCallable, Category = "Gorgeous Core|Common UI Foundation|Payload")
=======
	UFUNCTION(BlueprintCallable, Category = "Gorgeous UI|Payload")
>>>>>>> b4c134c (Some other changes i dont remember)
	static UPARAM(DisplayName = "Payload") FGorgeousUIUpdatePayload& AddGridToUIUpdate(UPARAM(ref) FGorgeousUIUpdatePayload& Payload, FName PropertyName, FGorgeousGridUpdatePayload Value);
};
