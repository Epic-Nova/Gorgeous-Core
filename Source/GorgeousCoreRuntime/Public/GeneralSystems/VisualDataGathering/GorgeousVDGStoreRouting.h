#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GorgeousVDGStoreRouting.generated.h"

/** Routing for the existing bundled VDG property store. */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousVDGStoreRouting : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    /** Use the local store unless both existing delegated systems are active. */
    UFUNCTION(BlueprintPure, Category="Gorgeous Things|Visual Data Gathering", meta=(WorldContext="WorldContextObject", DefaultToSelf="WorldContextObject"))
    static bool ShouldUseLocalVDGStore(const UObject* WorldContextObject);
};
