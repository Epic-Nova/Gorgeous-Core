#include "GeneralSystems/GorgeousSystemProjectSettings.h"

#include "GeneralSystems/GeneralSystemConfiguration_PDA.h"
#include "Misc/StringOutputDevice.h"
#include "UObject/Class.h"

FInstancedStruct UGorgeousSystemProjectSettings::Resolve(const UGeneralSystemConfiguration_PDA& Configuration)
{
	const FInstancedStruct& Defaults = Configuration.ProjectSettingsDefaults;
	const UScriptStruct* Schema = Defaults.GetScriptStruct();
	if (!Schema) return {};

	const auto* Override = GetDefault<UGorgeousSystemProjectSettings>()->Overrides.Find(FSoftObjectPath(&Configuration));
	if (!Override || Override->Schema != FSoftObjectPath(Schema)) return Defaults;

	FInstancedStruct Result = Defaults;
	FStringOutputDevice Errors;
	const TCHAR* End = Schema->ImportText(*Override->Values, Result.GetMutableMemory(), nullptr, PPF_None,
		&Errors, Schema->GetName(), false);
	if (!End || *End != 0 || !Errors.IsEmpty()) return Defaults;
	return Result;
}

#if WITH_EDITOR
bool UGorgeousSystemProjectSettings::SaveOverride(const UGeneralSystemConfiguration_PDA& Configuration, const FInstancedStruct& Values)
{
	const FInstancedStruct& Defaults = Configuration.ProjectSettingsDefaults;
	const UScriptStruct* Schema = Defaults.GetScriptStruct();
	if (!Schema || Schema != Values.GetScriptStruct()) return false;

	const FSoftObjectPath Key(&Configuration);
	FString Delta;
	Schema->ExportText(Delta, Values.GetMemory(), Defaults.GetMemory(), nullptr, PPF_None, nullptr, false);
	const TOptional<FGorgeousSystemProjectSettingsOverride> Previous = Overrides.Contains(Key)
		? TOptional<FGorgeousSystemProjectSettingsOverride>(Overrides.FindChecked(Key)) : TOptional<FGorgeousSystemProjectSettingsOverride>();
	if (Delta.IsEmpty() || Delta == TEXT("()"))
	{
		Overrides.Remove(Key);
	}
	else
	{
		auto& Entry = Overrides.FindOrAdd(Key);
		Entry.Schema = FSoftObjectPath(Schema);
		Entry.Values = MoveTemp(Delta);
	}

	if (TryUpdateDefaultConfigFile()) return true;
	if (Previous) Overrides.Add(Key, Previous.GetValue());
	else Overrides.Remove(Key);
	return false;
}
#endif
