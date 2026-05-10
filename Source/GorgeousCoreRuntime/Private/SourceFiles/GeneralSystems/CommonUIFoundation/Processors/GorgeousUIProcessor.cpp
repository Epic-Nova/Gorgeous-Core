// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/Processors/GorgeousUIProcessor.h"

#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "UObject/UObjectIterator.h"
#include "GeneralSystems/CommonUIFoundation/Interfaces/GorgeousUIWidget_I.h"
#include "GeneralSystems/CommonUIFoundation/DataAssets/GorgeousUITheme_DA.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSettings.h"
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"

void UGorgeousUIProcessor::OnSignalReceived(UObject* Widget, FGameplayTag SignalTag, const FInstancedStruct& Payload)
{
	if (!Widget) return;

	// Universal Reflection Magic:
	// The base implementation drives any UPROPERTY or Set-function on the widget by name.
	const FGorgeousUIUpdatePayload* UpdatePayload = Payload.GetPtr<FGorgeousUIUpdatePayload>();
	if (!UpdatePayload) return;

	for (const auto& Pair : UpdatePayload->Updates)
	{
		ApplyStylePropertyToTarget(Widget, Pair.Key, Pair.Value);
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

bool UGorgeousUIProcessor::ApplyStylePropertyToTarget(UObject* Target, FName PropertyName, const FInstancedStruct& Value)
{
	if (!IsStylePropertyAllowed(Target, PropertyName)) return false;
	return ApplyPropertyToTarget(Target, PropertyName, Value);
}

bool UGorgeousUIProcessor::IsStylePropertyAllowed(const UObject* Target, FName PropertyName)
{
	if (!Target || PropertyName.IsNone()) return false;

	if (const IGorgeousUIWidget_I* WidgetInterface = Cast<IGorgeousUIWidget_I>(Target))
	{
		if (!WidgetInterface->UseStylePropertyAllowList())
		{
			return true;
		}
		return WidgetInterface->GetStylePropertyAllowList().Contains(PropertyName);
	}

	return true;
}

const UGorgeousUITheme_DA* UGorgeousUIProcessor::GetDefaultTheme()
{
	const UGorgeousUIFoundationSettings* Settings = GetDefault<UGorgeousUIFoundationSettings>();
	if (!Settings || Settings->DefaultTheme.IsNull()) return nullptr;

	return Settings->DefaultTheme.LoadSynchronous();
}

void UGorgeousUIProcessor::ApplyThemeToWidgetInternal(UObject* Target, const UGorgeousUITheme_DA* PrimaryTheme, const UGorgeousUITheme_DA* FallbackTheme)
{
	if (!Target) return;

	IGorgeousUIWidget_I* WidgetInterface = Cast<IGorgeousUIWidget_I>(Target);
	if (!WidgetInterface) return;

	if (!PrimaryTheme && !FallbackTheme)
	{
		ApplyInterpolatedThemeColors(Target);
		return;
	}

	const UGorgeousUITheme_DA* DefaultTheme = FallbackTheme ? FallbackTheme : GetDefaultTheme();
	const FGameplayTag BindingTag = WidgetInterface->GetBindingTag();
	const bool bHasBindingTag = BindingTag.IsValid();

	TArray<FName> PropertiesToApply;
	const TSet<FName>& AllowedProperties = WidgetInterface->GetStylePropertyAllowList();
	if (WidgetInterface->UseStylePropertyAllowList())
	{
		if (AllowedProperties.Num() == 0)
		{
			return;
		}
		PropertiesToApply = AllowedProperties.Array();
	}
	else
	{
		auto AppendPropertiesFromTheme = [&](const UGorgeousUITheme_DA* Theme)
		{
			if (!Theme) return;
			const FString Prefix = bHasBindingTag ? (BindingTag.ToString() + TEXT(".")) : FString();
			for (const auto& Pair : Theme->StyleProperties)
			{
				const FString KeyString = Pair.Key.ToString();
				if (bHasBindingTag && KeyString.StartsWith(Prefix))
				{
					PropertiesToApply.AddUnique(FName(*KeyString.RightChop(Prefix.Len())));
					continue;
				}
				PropertiesToApply.AddUnique(Pair.Key);
			}
		};

		AppendPropertiesFromTheme(PrimaryTheme);
		AppendPropertiesFromTheme(DefaultTheme);
	}

	bool bHasThemeColors = false;

	for (const FName& PropertyName : PropertiesToApply)
	{
		FInstancedStruct ResolvedValue;
		if (!ResolveThemePropertyWithFallback(PrimaryTheme, DefaultTheme, BindingTag, PropertyName, ResolvedValue))
		{
			continue;
		}

		if (const FLinearColor* ColorPtr = ResolvedValue.GetPtr<FLinearColor>())
		{
			TMap<FName, FLinearColor>& StartColors = WidgetInterface->GetStartThemeColors();
			TMap<FName, FLinearColor>& TargetColors = WidgetInterface->GetTargetThemeColors();
			TMap<FName, FLinearColor>& CurrentColors = WidgetInterface->GetCurrentThemeColors();

			StartColors.Add(PropertyName, CurrentColors.FindOrAdd(PropertyName, *ColorPtr));
			TargetColors.Add(PropertyName, *ColorPtr);
			WidgetInterface->SetElapsedThemeTime(0.0f);
			WidgetInterface->SetIsInterpTheme(true);
			bHasThemeColors = true;
		}
		else
		{
			ApplyStylePropertyToTarget(Target, PropertyName, ResolvedValue);
		}
	}

	if (bHasThemeColors)
	{
		ApplyInterpolatedThemeColors(Target);
	}
}

#if WITH_EDITOR
void UGorgeousUIProcessor::ApplyEditorThemeToWidget(UObject* Target)
{
	const UGorgeousUITheme_DA* DefaultTheme = GetDefaultTheme();
	if (!DefaultTheme) return;

	if (Target)
	{
		if (UWorld* World = Target->GetWorld())
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				if (ULocalPlayer* LP = PC->GetLocalPlayer())
				{
					if (UGorgeousUIFoundationSubsystem* Subsystem = LP->GetSubsystem<UGorgeousUIFoundationSubsystem>())
					{
						Subsystem->ApplyThemeToWidget(Target, DefaultTheme);
						return;
					}
				}
			}
		}

		UClass* BestProcessorClass = nullptr;
		UClass* BestTargetClass = nullptr;
		TArray<UClass*> ProcessorDerivedClasses;
		GetDerivedClasses(UGorgeousUIProcessor::StaticClass(), ProcessorDerivedClasses);

		for (UClass* Class : ProcessorDerivedClasses)
		{
			if (!Class || Class->HasAnyClassFlags(CLASS_Abstract)) continue;
			const UGorgeousUIProcessor* ProcessorCDO = Class->GetDefaultObject<UGorgeousUIProcessor>();
			if (!ProcessorCDO || !ProcessorCDO->TargetWidgetClass) continue;

			if (Target->IsA(ProcessorCDO->TargetWidgetClass))
			{
				if (!BestTargetClass || ProcessorCDO->TargetWidgetClass->IsChildOf(BestTargetClass))
				{
					BestTargetClass = ProcessorCDO->TargetWidgetClass;
					BestProcessorClass = Class;
				}
			}
		}

		if (BestProcessorClass)
		{
			UGorgeousUIProcessor* Processor = NewObject<UGorgeousUIProcessor>(GetTransientPackage(), BestProcessorClass);
			if (Processor)
			{
				Processor->ApplyThemeToWidget(Target, DefaultTheme);
				return;
			}
		}
	}

	ApplyThemeToWidgetInternal(Target, DefaultTheme, nullptr);
}
#endif

void UGorgeousUIProcessor::ApplyThemeToWidget(UObject* Widget, const UGorgeousUITheme_DA* PrimaryTheme, const UGorgeousUITheme_DA* FallbackTheme)
{
	ApplyThemeToWidgetInternal(Widget, PrimaryTheme, FallbackTheme);
}

bool UGorgeousUIProcessor::ResolveThemeProperty(const UGorgeousUITheme_DA* Theme, FName Key, FInstancedStruct& OutValue)
{
	if (!Theme || Key.IsNone()) return false;

	if (const FInstancedStruct* Value = Theme->StyleProperties.Find(Key))
	{
		OutValue = *Value;
		return true;
	}

	return false;
}

bool UGorgeousUIProcessor::ResolveThemePropertyWithFallback(const UGorgeousUITheme_DA* PrimaryTheme, const UGorgeousUITheme_DA* FallbackTheme, const FGameplayTag& BindingTag, FName PropertyName, FInstancedStruct& OutValue)
{
	const bool bHasBindingTag = BindingTag.IsValid();
	const FString PropertyNameString = PropertyName.ToString();

	if (bHasBindingTag)
	{
		const FString PrefixedKeyString = FString::Printf(TEXT("%s.%s"), *BindingTag.ToString(), *PropertyNameString);
		if (ResolveThemeProperty(PrimaryTheme, FName(*PrefixedKeyString), OutValue)) return true;
	}

	if (ResolveThemeProperty(PrimaryTheme, PropertyName, OutValue)) return true;

	if (FallbackTheme && FallbackTheme != PrimaryTheme)
	{
		if (bHasBindingTag)
		{
			const FString PrefixedKeyString = FString::Printf(TEXT("%s.%s"), *BindingTag.ToString(), *PropertyNameString);
			if (ResolveThemeProperty(FallbackTheme, FName(*PrefixedKeyString), OutValue)) return true;
		}
		if (ResolveThemeProperty(FallbackTheme, PropertyName, OutValue)) return true;
	}

	return false;
}

void UGorgeousUIProcessor::ApplyInterpolatedThemeColors(UObject* Target)
{
	IGorgeousUIWidget_I* WidgetInterface = Cast<IGorgeousUIWidget_I>(Target);
	if (!WidgetInterface) return;

	TMap<FName, FLinearColor>& TargetColors = WidgetInterface->GetTargetThemeColors();
	TMap<FName, FLinearColor>& CurrentColors = WidgetInterface->GetCurrentThemeColors();

	for (const auto& Pair : TargetColors)
	{
		const FName PropertyName = Pair.Key;
		if (!IsStylePropertyAllowed(Target, PropertyName))
		{
			continue;
		}

		const FLinearColor& ColorValue = CurrentColors.Contains(PropertyName) ? CurrentColors[PropertyName] : Pair.Value;
		FInstancedStruct ColorPayload;
		ColorPayload.InitializeAs<FLinearColor>(ColorValue);
		ApplyStylePropertyToTarget(Target, PropertyName, ColorPayload);
	}
}
