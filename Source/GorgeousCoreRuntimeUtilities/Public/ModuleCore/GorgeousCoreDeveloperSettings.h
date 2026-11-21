#pragma once

#include "Engine/DeveloperSettings.h"
#include "GorgeousCoreDeveloperSettings.generated.h"

/** Settings discovered by the Gorgeous Core project settings pages. */
UCLASS(Abstract)
class GORGEOUSCORERUNTIMEUTILITIES_API UGorgeousCoreDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	explicit UGorgeousCoreDeveloperSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
	{
	}

	virtual FName GetCategoryName() const override { return TEXT("Gorgeous Things"); }

#if WITH_EDITOR
	virtual bool SupportsAutoRegistration() const override { return false; }
#endif
};
