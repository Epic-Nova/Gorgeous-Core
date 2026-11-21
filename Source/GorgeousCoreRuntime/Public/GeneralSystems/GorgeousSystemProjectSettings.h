#pragma once

#include "ModuleCore/GorgeousCoreDeveloperSettings.h"
#include "UObject/SoftObjectPath.h"
#include "GorgeousSystemProjectSettings.generated.h"

struct FInstancedStruct;
class UGeneralSystemConfiguration_PDA;

USTRUCT()
struct FGorgeousSystemProjectSettingsOverride
{
	GENERATED_BODY()

	UPROPERTY()
	FSoftObjectPath Schema;

	UPROPERTY()
	FString Values;
};

/** Stores overrides separately from the system's authored asset defaults. */
UCLASS(Config = Game, DefaultConfig, meta = (GorgeousSettingsPage = "Hidden"))
class GORGEOUSCORERUNTIME_API UGorgeousSystemProjectSettings : public UGorgeousCoreDeveloperSettings
{
	GENERATED_BODY()

public:
	static FInstancedStruct Resolve(const UGeneralSystemConfiguration_PDA& Configuration);

#if WITH_EDITOR
	bool SaveOverride(const UGeneralSystemConfiguration_PDA& Configuration, const FInstancedStruct& Values);
#endif

private:
	UPROPERTY(Config)
	TMap<FSoftObjectPath, FGorgeousSystemProjectSettingsOverride> Overrides;
};
