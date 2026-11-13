#pragma once
#include "Subsystems/WorldSubsystem.h"
#include "GeneralSystems/SignalBridge/SignalBridgeStorage_OV.h"
#include "GorgeousSignalBridgeWorldSubsystem.generated.h"

/** Holds local signal delegates for exactly one world's lifetime. */
UCLASS()
class GORGEOUSCORERUNTIME_API UGorgeousSignalBridgeWorldSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    USignalBridgeStorage_OV* GetStorage()
    {
        if (!Storage)
        {
            Storage = NewObject<USignalBridgeStorage_OV>(this);
            Storage->SetFallbackOwner(GetWorld());
        }
        return Storage;
    }
private:
    UPROPERTY(Transient)
    TObjectPtr<USignalBridgeStorage_OV> Storage;
};
