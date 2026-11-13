// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once

//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "Templates/Casts.h"
#include "Misc/EngineVersion.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "Runtime/Launch/Resources/Version.h"
//<-------------------------------------------------------------------------->

/*======================================================================================================================>
| --------------------------------------------------------------------------------------------------------------------- |
|													MACRO HELL															                                                                                 |
| --------------------------------------------------------------------------------------------------------------------- |
<======================================================================================================================*/

//@TODO: Reconsider to drop the planned support for UE 4.23 - 4.27 and only support UE 5.0 and higher going forward, depending on the communities needs.
//@TODO: concider removing this class all-together as we dont want re reinvent the weel (@see UE_VERSION_NEWER_THAN & UE_VERSION_OLDER_THAN macros)

/**
 * Multiplies the major engine version by 100 and adds the minor version for better usage in the macros below.
 */
#define GORGEOUS_ENGINE_VERSION (ENGINE_MAJOR_VERSION * 100 + ENGINE_MINOR_VERSION)

// HIGHER and LOWER include the named version. ONLY matches exactly.

#if GORGEOUS_ENGINE_VERSION >= 423
#define GORGEOUS_23_HIGHER(...) __VA_ARGS__
#else
#define GORGEOUS_23_HIGHER(...)
#endif

#if GORGEOUS_ENGINE_VERSION <= 423
#define GORGEOUS_23_LOWER(...) __VA_ARGS__
#else
#define GORGEOUS_23_LOWER(...)
#endif

#if GORGEOUS_ENGINE_VERSION == 423
#define GORGEOUS_23_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_23_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 423
#define GORGEOUS_23_SWITCH(Before, AfterEqual) AfterEqual
#else
#define GORGEOUS_23_SWITCH(Before, AfterEqual) Before
#endif

#if GORGEOUS_ENGINE_VERSION >= 424
#define GORGEOUS_24_HIGHER(...) __VA_ARGS__
#else
#define GORGEOUS_24_HIGHER(...)
#endif

#if GORGEOUS_ENGINE_VERSION <= 424
#define GORGEOUS_24_LOWER(...) __VA_ARGS__
#else
#define GORGEOUS_24_LOWER(...)
#endif

#if GORGEOUS_ENGINE_VERSION == 424
#define GORGEOUS_24_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_24_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 424
#define GORGEOUS_24_SWITCH(Before, AfterEqual) AfterEqual
#else
#define GORGEOUS_24_SWITCH(Before, AfterEqual) Before
#endif

#if GORGEOUS_ENGINE_VERSION >= 425
#define GORGEOUS_25_HIGHER(...) __VA_ARGS__
#else
#define GORGEOUS_25_HIGHER(...)
#endif

#if GORGEOUS_ENGINE_VERSION <= 425
#define GORGEOUS_25_LOWER(...) __VA_ARGS__
#else
#define GORGEOUS_25_LOWER(...)
#endif

#if GORGEOUS_ENGINE_VERSION == 425
#define GORGEOUS_25_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_25_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 425
#define GORGEOUS_25_SWITCH(Before, AfterEqual) AfterEqual
#else
#define GORGEOUS_25_SWITCH(Before, AfterEqual) Before
#endif

#if GORGEOUS_ENGINE_VERSION >= 426
#define GORGEOUS_26_HIGHER(...) __VA_ARGS__
#else
#define GORGEOUS_26_HIGHER(...)
#endif

#if GORGEOUS_ENGINE_VERSION <= 426
#define GORGEOUS_26_LOWER(...) __VA_ARGS__
#else
#define GORGEOUS_26_LOWER(...)
#endif

#if GORGEOUS_ENGINE_VERSION == 426
#define GORGEOUS_26_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_26_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 426
#define GORGEOUS_26_SWITCH(Before, AfterEqual) AfterEqual
#else
#define GORGEOUS_26_SWITCH(Before, AfterEqual) Before
#endif

#if GORGEOUS_ENGINE_VERSION >= 427
#define GORGEOUS_27_HIGHER(...) __VA_ARGS__
#else
#define GORGEOUS_27_HIGHER(...)
#endif

#if GORGEOUS_ENGINE_VERSION <= 427
#define GORGEOUS_27_LOWER(...) __VA_ARGS__
#else
#define GORGEOUS_27_LOWER(...)
#endif

#if GORGEOUS_ENGINE_VERSION == 427
#define GORGEOUS_27_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_27_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 427
#define GORGEOUS_27_SWITCH(Before, AfterEqual) AfterEqual
#else
#define GORGEOUS_27_SWITCH(Before, AfterEqual) Before
#endif

#if GORGEOUS_ENGINE_VERSION >= 500
#define GORGEOUS_5_HIGHER(...) __VA_ARGS__
#else
#define GORGEOUS_5_HIGHER(...)
#endif

#if GORGEOUS_ENGINE_VERSION <= 500
#define GORGEOUS_5_LOWER(...) __VA_ARGS__
#else
#define GORGEOUS_5_LOWER(...)
#endif

#if GORGEOUS_ENGINE_VERSION == 500
#define GORGEOUS_5_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_5_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 500
#define GORGEOUS_5_SWITCH(Before, AfterEqual) AfterEqual
#else
#define GORGEOUS_5_SWITCH(Before, AfterEqual) Before
#endif

#if GORGEOUS_ENGINE_VERSION >= 501
#define GORGEOUS_51_HIGHER(...) __VA_ARGS__
#else
#define GORGEOUS_51_HIGHER(...)
#endif

#if GORGEOUS_ENGINE_VERSION <= 501
#define GORGEOUS_51_LOWER(...) __VA_ARGS__
#else
#define GORGEOUS_51_LOWER(...)
#endif

#if GORGEOUS_ENGINE_VERSION == 501
#define GORGEOUS_51_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_51_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 501
#define GORGEOUS_51_SWITCH(Before, AfterEqual) AfterEqual
#else
#define GORGEOUS_51_SWITCH(Before, AfterEqual) Before
#endif

#if GORGEOUS_ENGINE_VERSION >= 502
#define GORGEOUS_52_HIGHER(...) __VA_ARGS__
#else
#define GORGEOUS_52_HIGHER(...)
#endif

#if GORGEOUS_ENGINE_VERSION <= 502
#define GORGEOUS_52_LOWER(...) __VA_ARGS__
#else
#define GORGEOUS_52_LOWER(...)
#endif

#if GORGEOUS_ENGINE_VERSION == 502
#define GORGEOUS_52_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_52_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 502
#define GORGEOUS_52_SWITCH(Before, AfterEqual) AfterEqual
#else
#define GORGEOUS_52_SWITCH(Before, AfterEqual) Before
#endif

#if GORGEOUS_ENGINE_VERSION >= 503
#define GORGEOUS_53_HIGHER(...) __VA_ARGS__
#else
#define GORGEOUS_53_HIGHER(...)
#endif

#if GORGEOUS_ENGINE_VERSION <= 503
#define GORGEOUS_53_LOWER(...) __VA_ARGS__
#else
#define GORGEOUS_53_LOWER(...)
#endif

#if GORGEOUS_ENGINE_VERSION == 503
#define GORGEOUS_53_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_53_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 503
#define GORGEOUS_53_SWITCH(Before, AfterEqual) AfterEqual
#else
#define GORGEOUS_53_SWITCH(Before, AfterEqual) Before
#endif

#if GORGEOUS_ENGINE_VERSION >= 504
#define GORGEOUS_54_HIGHER(...) __VA_ARGS__
#else
#define GORGEOUS_54_HIGHER(...)
#endif

#if GORGEOUS_ENGINE_VERSION <= 504
#define GORGEOUS_54_LOWER(...) __VA_ARGS__
#else
#define GORGEOUS_54_LOWER(...)
#endif

#if GORGEOUS_ENGINE_VERSION == 504
#define GORGEOUS_54_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_54_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 504
#define GORGEOUS_54_SWITCH(Before, AfterEqual) AfterEqual
#else
#define GORGEOUS_54_SWITCH(Before, AfterEqual) Before
#endif

#if GORGEOUS_ENGINE_VERSION >= 505
#define GORGEOUS_55_HIGHER(...) __VA_ARGS__
#else
#define GORGEOUS_55_HIGHER(...)
#endif

#if GORGEOUS_ENGINE_VERSION <= 505
#define GORGEOUS_55_LOWER(...) __VA_ARGS__
#else
#define GORGEOUS_55_LOWER(...)
#endif

#if GORGEOUS_ENGINE_VERSION == 505
#define GORGEOUS_55_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_55_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 505
#define GORGEOUS_55_SWITCH(Before, AfterEqual) AfterEqual
#else
#define GORGEOUS_55_SWITCH(Before, AfterEqual) Before
#endif

#if GORGEOUS_ENGINE_VERSION >= 506
#define GORGEOUS_56_HIGHER(...) __VA_ARGS__
#else
#define GORGEOUS_56_HIGHER(...)
#endif

#if GORGEOUS_ENGINE_VERSION <= 506
#define GORGEOUS_56_LOWER(...) __VA_ARGS__
#else
#define GORGEOUS_56_LOWER(...)
#endif

#if GORGEOUS_ENGINE_VERSION == 506
#define GORGEOUS_56_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_56_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 506
#define GORGEOUS_56_SWITCH(Before, AfterEqual) AfterEqual
#else
#define GORGEOUS_56_SWITCH(Before, AfterEqual) Before
#endif

#if GORGEOUS_ENGINE_VERSION >= 507
#define GORGEOUS_57_HIGHER(...) __VA_ARGS__
#else
#define GORGEOUS_57_HIGHER(...)
#endif

#if GORGEOUS_ENGINE_VERSION <= 507
#define GORGEOUS_57_LOWER(...) __VA_ARGS__
#else
#define GORGEOUS_57_LOWER(...)
#endif

#if GORGEOUS_ENGINE_VERSION == 507
#define GORGEOUS_57_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_57_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 507
#define GORGEOUS_57_SWITCH(Before, AfterEqual) AfterEqual
#else
#define GORGEOUS_57_SWITCH(Before, AfterEqual) Before
#endif

#if GORGEOUS_ENGINE_VERSION >= 508
#define GORGEOUS_58_HIGHER(...) __VA_ARGS__
#else
#define GORGEOUS_58_HIGHER(...)
#endif

#if GORGEOUS_ENGINE_VERSION <= 508
#define GORGEOUS_58_LOWER(...) __VA_ARGS__
#else
#define GORGEOUS_58_LOWER(...)
#endif

#if GORGEOUS_ENGINE_VERSION == 508
#define GORGEOUS_58_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_58_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 508
#define GORGEOUS_58_SWITCH(Before, AfterEqual) AfterEqual
#else
#define GORGEOUS_58_SWITCH(Before, AfterEqual) Before
#endif
/**
* If the Unreal Engine version is below 4.25 then F is referred to U.
*/
#if GORGEOUS_ENGINE_VERSION < 425
using FProperty = UProperty;
using FStructProperty = UStructProperty;
using FSoftObjectProperty = USoftObjectProperty;
using FIntProperty = UIntProperty;
using FFloatProperty = UFloatProperty;
using FBoolProperty = UBoolProperty;
using FNameProperty = UNameProperty;
using FObjectProperty = UObjectProperty;
using FStructProperty = UStructProperty;
using FArrayProperty = UArrayProperty;
using FMapProperty = UMapProperty;
using FSetProperty = USetProperty;
#endif

/**
 * If the Unreal Engine version is below 5, TObjectPtr is referred to *.
 */
#if GORGEOUS_ENGINE_VERSION < 500
template<typename T>
using TObjectPtr = T*;
#endif