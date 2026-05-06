// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"
#include "Blueprint/UserWidget.h"

void UGorgeousUIProcessor::OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	if (!Widget) return;

	// Universal Reflection Magic:
	// The base implementation drives any UPROPERTY or Set-function on the widget by name.
	const FGorgeousUIUpdatePayload* UpdatePayload = Payload.GetPtr<FGorgeousUIUpdatePayload>();
	if (!UpdatePayload) return;

	for (const auto& Pair : UpdatePayload->Updates)
	{
		ApplyPropertyToTarget(Widget, Pair.Key, Pair.Value);
	}
}

bool UGorgeousUIProcessor::ApplyPropertyToTarget(UObject* Target, FName PropertyName, const FInstancedStruct& Value)
{
	if (!Target || PropertyName.IsNone()) return false;

	UClass* TargetClass = Target->GetClass();
	const UScriptStruct* ValueStruct = Value.GetScriptStruct();
	if (!ValueStruct) return false;

	const void* ValueMemory = Value.GetMemory();

	// 1. Direct UPROPERTY match by name.
	if (FProperty* Prop = TargetClass->FindPropertyByName(PropertyName))
	{
		// Guard: only copy if the sizes match to avoid memory corruption.
		if (Prop->ElementSize == ValueStruct->GetStructureSize())
		{
			Prop->CopyCompleteValue(Prop->ContainerPtrToValuePtr<void>(Target), ValueMemory);
			return true;
		}
	}

	// 2. Setter function match ("Percent" -> SetPercent, "Text" -> SetText, etc.).
	const FString SetterName = FString::Printf(TEXT("Set%s"), *PropertyName.ToString());
	if (UFunction* SetterFunc = TargetClass->FindFunctionByName(*SetterName))
	{
		// We only handle functions with exactly one input parameter (return values are fine).
		// Count actual input params — skip return value properties.
		int32 InputParamCount = 0;
		for (TFieldIterator<FProperty> It(SetterFunc); It && (It->PropertyFlags & CPF_Parm); ++It)
		{
			if (!(It->PropertyFlags & CPF_ReturnParm))
			{
				++InputParamCount;
			}
		}

		if (InputParamCount == 1)
		{
			// Build a correctly-sized parameter buffer for ProcessEvent.
			// ProcessEvent expects the full Parms struct, not just the raw value.
			TArray<uint8> ParmsBuffer;
			ParmsBuffer.SetNumZeroed(SetterFunc->ParmsSize);

			// Copy our value into the start of the parms buffer.
			FMemory::Memcpy(ParmsBuffer.GetData(), ValueMemory, FMath::Min<int32>(SetterFunc->ParmsSize, (int32)ValueStruct->GetStructureSize()));

			Target->ProcessEvent(SetterFunc, ParmsBuffer.GetData());
			return true;
		}
	}

	// 3. Boolean prefix strip: "bIsVisible" -> retry as "IsVisible".
	const FString PropStr = PropertyName.ToString();
	if (PropStr.Len() > 1 && PropStr.StartsWith(TEXT("b")) && FChar::IsUpper(PropStr[1]))
	{
		return ApplyPropertyToTarget(Target, *PropStr.RightChop(1), Value);
	}

	return false;
}
