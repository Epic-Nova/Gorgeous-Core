// Copyright (c) 2026 Simsalabim Studios. All rights reserved.
#pragma once
#include "CoreTypes.h"
#include "Misc/EngineVersionComparison.h"

// Native API verified in installed UE5.7 and UE5.8; older/stripped builds use the fallback.
#if UE_WITH_IRIS && GORGEOUSCORE_WITH_IRIS && UE_VERSION_NEWER_THAN(5,6,99) && __has_include("Iris/ReplicationSystem/ReplicationFragmentUtil.h") && __has_include("Net/Iris/ReplicationSystem/ReplicationSystemUtil.h")
#define GORGEOUS_NATIVE_IRIS_SUPPORTED 1
#else
#define GORGEOUS_NATIVE_IRIS_SUPPORTED 0
#endif
