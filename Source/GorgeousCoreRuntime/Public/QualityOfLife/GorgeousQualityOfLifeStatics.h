#pragma once

#include "CoreMinimal.h"
#include "AutoReplication/GorgeousAutoReplicationMixin.h"

class UObject_AOTOV;

namespace FGorgeousQualityOfLifeStatics
{
	GORGEOUSCORERUNTIME_API UObject_AOTOV* EnsureSelfReference(UObject* Owner, TMap<FName, FGorgeousAutoReplicationEntry>& AdditionalData, bool bExposeThroughNetworkStack);
	GORGEOUSCORERUNTIME_API void ClearSelfReference(const UObject* Owner, TMap<FName, FGorgeousAutoReplicationEntry>& AdditionalData);
	GORGEOUSCORERUNTIME_API UObject* ResolveSelfReference(TSubclassOf<UObject> QualityOfLifeClass);
	GORGEOUSCORERUNTIME_API FString ResolveSelfReferenceName(TSubclassOf<UObject> QualityOfLifeClass);
	
	/** Strips any default object variables from a CDO so they don't end up serialized into cooked assets. */
	GORGEOUSCORERUNTIME_API void SanitizeCDOAdditionalData(UObject* Owner, TMap<FName, FGorgeousAutoReplicationEntry>& AdditionalData);
}
