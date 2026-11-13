#pragma once
#include "CoreMinimal.h"

/** Soft per-stream byte budget: a single oversized update may incur debt, never be dropped. */
struct FGorgeousReplicationTokenBucket
{
    double AvailableBytes=0;
    bool bInitialized=false;
    void Refill(double DeltaSeconds,double KilobytesPerSecond)
    {
        const double Rate=FMath::Max(0.0,KilobytesPerSecond)*1024.0;
        if (!bInitialized) { AvailableBytes=Rate; bInitialized=true; }
        else AvailableBytes=FMath::Min(Rate,AvailableBytes+FMath::Max(0.0,DeltaSeconds)*Rate);
    }
    bool CanDispatch(double KilobytesPerSecond) const { return KilobytesPerSecond<=0.0 || AvailableBytes>0.0; }
    void Consume(int64 Bytes) { AvailableBytes-=FMath::Max<int64>(0,Bytes); }
};
