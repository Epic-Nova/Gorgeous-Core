// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationLibrary.h"
#include "StructUtils/InstancedStruct.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIInstancedValueUtils.h"

#include "GeneralSystems/CommonUIFoundation/GorgeousPrimaryGameLayout.h"
#include "CommonActivatableWidget.h"

UGorgeousPrimaryGameLayout* UGorgeousUIFoundationLibrary::GetGorgeousPrimaryLayout(const UObject* WorldContextObject)
{
	return UGorgeousPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(WorldContextObject);
}

UGorgeousPrimaryGameLayout* UGorgeousUIFoundationLibrary::GetGorgeousPrimaryLayoutFromController(APlayerController* PlayerController)
{
	return UGorgeousPrimaryGameLayout::GetPrimaryGameLayout(PlayerController);
}

void UGorgeousUIFoundationLibrary::FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	if (ActivatableWidget)
	{
		if (UGorgeousPrimaryGameLayout* Layout = GetGorgeousPrimaryLayoutFromController(ActivatableWidget->GetOwningPlayer()))
		{
			Layout->FindAndRemoveWidgetFromLayer(ActivatableWidget);
		}
	}
}

UCommonActivatableWidgetContainerBase* UGorgeousUIFoundationLibrary::GetLayerWidget(const UObject* WorldContextObject, FGameplayTag LayerTag)
{
	if (UGorgeousPrimaryGameLayout* Layout = GetGorgeousPrimaryLayout(WorldContextObject))
	{
		return Layout->GetLayerWidget(LayerTag);
	}
	return nullptr;
}

FGorgeousUIUpdatePayload UGorgeousUIFoundationLibrary::MakeGorgeousUIUpdatePayload()
{
	return FGorgeousUIUpdatePayload();
}

FGorgeousUIUpdatePayload& UGorgeousUIFoundationLibrary::AddTextToUIUpdate(FGorgeousUIUpdatePayload& Payload, FName PropertyName, FText Value)
{
	FGorgeousInstancedText Wrapper;
	Wrapper.Value = Value;
	FInstancedStruct InstancedValue = FInstancedStruct::Make(Wrapper);
	Payload.Updates.Add(PropertyName, InstancedValue);
	return Payload;
}

FGorgeousUIUpdatePayload& UGorgeousUIFoundationLibrary::AddColorToUIUpdate(FGorgeousUIUpdatePayload& Payload, FName PropertyName, FLinearColor Value)
{
	FInstancedStruct InstancedValue;
	InstancedValue.InitializeAs<FLinearColor>(Value);
	Payload.Updates.Add(PropertyName, InstancedValue);
	return Payload;
}

FGorgeousUIUpdatePayload& UGorgeousUIFoundationLibrary::AddFloatToUIUpdate(FGorgeousUIUpdatePayload& Payload, FName PropertyName, float Value)
{
	FGorgeousInstancedFloat Wrapper;
	Wrapper.Value = Value;
	FInstancedStruct InstancedValue = FInstancedStruct::Make(Wrapper);
	Payload.Updates.Add(PropertyName, InstancedValue);
	return Payload;
}

FGorgeousUIUpdatePayload& UGorgeousUIFoundationLibrary::AddIntToUIUpdate(FGorgeousUIUpdatePayload& Payload, FName PropertyName, int32 Value)
{
	FGorgeousInstancedInt Wrapper;
	Wrapper.Value = Value;
	FInstancedStruct InstancedValue = FInstancedStruct::Make(Wrapper);
	Payload.Updates.Add(PropertyName, InstancedValue);
	return Payload;
}

FGorgeousUIUpdatePayload& UGorgeousUIFoundationLibrary::AddObjectToUIUpdate(FGorgeousUIUpdatePayload& Payload, FName PropertyName, UObject* Value)
{
	FGorgeousInstancedObject Wrapper;
	Wrapper.Value = Value;
	FInstancedStruct InstancedValue = FInstancedStruct::Make(Wrapper);
	Payload.Updates.Add(PropertyName, InstancedValue);
	return Payload;
}

FGorgeousUIUpdatePayload& UGorgeousUIFoundationLibrary::AddBoolToUIUpdate(FGorgeousUIUpdatePayload& Payload, FName PropertyName, bool Value)
{
	FGorgeousInstancedBool Wrapper;
	Wrapper.Value = Value;
	FInstancedStruct InstancedValue = FInstancedStruct::Make(Wrapper);
	Payload.Updates.Add(PropertyName, InstancedValue);
	return Payload;
}

FGorgeousUIUpdatePayload& UGorgeousUIFoundationLibrary::AddPaddingToUIUpdate(FGorgeousUIUpdatePayload& Payload, FName PropertyName, FMargin Value)
{
	FInstancedStruct InstancedValue;
	InstancedValue.InitializeAs<FMargin>(Value);
	Payload.Updates.Add(PropertyName, InstancedValue);
	return Payload;
}

FGorgeousUIUpdatePayload& UGorgeousUIFoundationLibrary::AddGridToUIUpdate(FGorgeousUIUpdatePayload& Payload, FName PropertyName, FGorgeousGridUpdatePayload Value)
{
	FInstancedStruct InstancedValue;
	InstancedValue.InitializeAs<FGorgeousGridUpdatePayload>(Value);
	Payload.Updates.Add(PropertyName, InstancedValue);
	return Payload;
}
