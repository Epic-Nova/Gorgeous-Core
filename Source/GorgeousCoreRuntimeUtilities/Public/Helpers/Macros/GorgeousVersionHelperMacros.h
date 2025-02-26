// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|      that is has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/
#pragma once

#include "Templates/Casts.h"
#include "Misc/EngineVersion.h"

#define GORGEOUS_ENGINE_VERSION (ENGINE_MAJOR_VERSION * 100 + ENGINE_MINOR_VERSION)

// ---------------------------------------------------------------------------------------------------------------------//
//																														//
// If the Unreal Engine version is higher than to the defined engine version, then these macros are active.	            //
//																														//
// ---------------------------------------------------------------------------------------------------------------------//	
#if GORGEOUS_ENGINE_VERSION < 423
#define GORGEOUS_23_HIGHER(...)
#else
#define GORGEOUS_23_HIGHER(...) __VA_ARGS__	
#endif

#if GORGEOUS_ENGINE_VERSION < 424
#define GORGEOUS_24_HIGHER(...)
#else
#define GORGEOUS_24_HIGHER(...) __VA_ARGS__
#endif

#if GORGEOUS_ENGINE_VERSION < 425
#define GORGEOUS_25_HIGHER(...)
#else
#define GORGEOUS_25_HIGHER(...) __VA_ARGS__
#endif

#if GORGEOUS_ENGINE_VERSION < 426
#define GORGEOUS_26_HIGHER(...)
#else
#define GORGEOUS_26_HIGHER(...) __VA_ARGS__
#endif

#if GORGEOUS_ENGINE_VERSION < 427
#define GORGEOUS_27_HIGHER(...)
#else
#define GORGEOUS_27_HIGHER(...) __VA_ARGS__
#endif

#if GORGEOUS_ENGINE_VERSION < 500
#define GORGEOUS_5_HIGHER(...)
#else
#define GORGEOUS_5_HIGHER(...) __VA_ARGS__
#endif

#if GORGEOUS_ENGINE_VERSION < 510
#define GORGEOUS_51_HIGHER(...)
#else
#define GORGEOUS_51_HIGHER(...) __VA_ARGS__
#endif

#if GORGEOUS_ENGINE_VERSION < 520
#define GORGEOUS_52_HIGHER(...)
#else
#define GORGEOUS_52_HIGHER(...) __VA_ARGS__
#endif

#if GORGEOUS_ENGINE_VERSION < 530
#define GORGEOUS_53_HIGHER(...)
#else
#define GORGEOUS_53_HIGHER(...) __VA_ARGS__
#endif

#if GORGEOUS_ENGINE_VERSION < 540
#define GORGEOUS_54_HIGHER(...)
#else
#define GORGEOUS_54_HIGHER(...) __VA_ARGS__
#endif


// -----------------------------------------------------------------------------------------------------------------//
//																													//
// If the Unreal Engine version is lower than to the defined engine version, then these macros are active.	        //
//																													//
// -----------------------------------------------------------------------------------------------------------------//	
#if GORGEOUS_ENGINE_VERSION > 423
#define GORGEOUS_23_LOWER(X)
#else
#define GORGEOUS_23_LOWER(...) __VA_ARGS__
#endif

#if GORGEOUS_ENGINE_VERSION > 424
#define GORGEOUS_24_LOWER(X)
#else
#define GORGEOUS_24_LOWER(...) __VA_ARGS__
#endif

#if GORGEOUS_ENGINE_VERSION > 425
#define GORGEOUS_25_LOWER(X)
#else
#define GORGEOUS_25_LOWER(...) __VA_ARGS__
#endif

#if GORGEOUS_ENGINE_VERSION > 426
#define GORGEOUS_26_LOWER(X)
#else
#define GORGEOUS_26_LOWER(...) __VA_ARGS__
#endif

#if GORGEOUS_ENGINE_VERSION > 427
#define GORGEOUS_27_LOWER(X)
#else
#define GORGEOUS_27_LOWER(...) __VA_ARGS__
#endif

#if GORGEOUS_ENGINE_VERSION > 500
#define GORGEOUS_5_LOWER(X)
#else
#define GORGEOUS_5_LOWER(...) __VA_ARGS__
#endif

#if GORGEOUS_ENGINE_VERSION > 510
#define GORGEOUS_51_LOWER(X)
#else
#define GORGEOUS_51_LOWER(...) __VA_ARGS__
#endif

#if GORGEOUS_ENGINE_VERSION > 520
#define GORGEOUS_52_LOWER(X)
#else
#define GORGEOUS_52_LOWER(...) __VA_ARGS__
#endif

#if GORGEOUS_ENGINE_VERSION > 530
#define GORGEOUS_53_LOWER(X)
#else
#define GORGEOUS_53_LOWER(...) __VA_ARGS__
#endif

#if GORGEOUS_ENGINE_VERSION > 540
#define GORGEOUS_54_LOWER(X)
#else
#define GORGEOUS_54_LOWER(...) __VA_ARGS__
#endif

#if GORGEOUS_ENGINE_VERSION > 550
#define GORGEOUS_55_LOWER(X)
#else
#define GORGEOUS_55_LOWER(...) __VA_ARGS__
#endif

// -------------------------------------------------------------------------------------------------------------------------//
//																															//
// If the Unreal Engine version is higher than or equal to the defined engine version, then the Switch macros are active.	//
// If the Unreal Engine version is the same as the defined engine version, the only macros are active.						//
//																															//
// -------------------------------------------------------------------------------------------------------------------------//

#if GORGEOUS_ENGINE_VERSION >= 423
#define GORGEOUS_23_SWITCH(Before, AfterEqual) AfterEqual
#define GORGEOUS_23_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_23_SWITCH(Before, AfterEqual) Before
#define GORGEOUS_23_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 424
#define GORGEOUS_24_SWITCH(Before, AfterEqual) AfterEqual
#define GORGEOUS_24_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_24_SWITCH(Before, AfterEqual) Before
#define GORGEOUS_24_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 425
#define GORGEOUS_25_SWITCH(Before, AfterEqual) AfterEqual
#define GORGEOUS_25_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_25_SWITCH(Before, AfterEqual) Before
#define GORGEOUS_25_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 426
#define GORGEOUS_26_SWITCH(Before, AfterEqual) AfterEqual
#define GORGEOUS_26_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_26_SWITCH(Before, AfterEqual) Before
#define GORGEOUS_26_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 427
#define GORGEOUS_27_SWITCH(Before, AfterEqual) AfterEqual
#define GORGEOUS_27_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_27_SWITCH(Before, AfterEqual) Before
#define GORGEOUS_27_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 500
#define GORGEOUS_5_SWITCH(Before, AfterEqual) AfterEqual
#define GORGEOUS_5_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_5_SWITCH(Before, AfterEqual) Before
#define GORGEOUS_5_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 510
#define GORGEOUS_51_SWITCH(Before, AfterEqual) AfterEqual
#define GORGEOUS_51_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_51_SWITCH(Before, AfterEqual) Before
#define GORGEOUS_51_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 520
#define GORGEOUS_52_SWITCH(Before, AfterEqual) AfterEqual
#define GORGEOUS_52_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_52_SWITCH(Before, AfterEqual) Before
#define GORGEOUS_52_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 530
#define GORGEOUS_53_SWITCH(Before, AfterEqual) AfterEqual
#define GORGEOUS_53_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_53_SWITCH(Before, AfterEqual) Before
#define GORGEOUS_53_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 540
#define GORGEOUS_54_SWITCH(Before, AfterEqual) AfterEqual
#define GORGEOUS_54_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_54_SWITCH(Before, AfterEqual) Before
#define GORGEOUS_54_ONLY(...)
#endif

#if GORGEOUS_ENGINE_VERSION >= 550
#define GORGEOUS_55_SWITCH(Before, AfterEqual) AfterEqual
#define GORGEOUS_55_ONLY(...) __VA_ARGS__
#else
#define GORGEOUS_55_SWITCH(Before, AfterEqual) Before
#define GORGEOUS_55_ONLY(...)
#endif

// ---------------------------------------------------------------------//
//																		//
// If the Unreal Engine version is below 4.25 then F is referred to U.	//
//																		//
// ---------------------------------------------------------------------//
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

// -------------------------------------------------------------------------//
//																			//
// If the Unreal Engine version is below 5, TObjectPtr is referred to *.	//
//																			//
// -------------------------------------------------------------------------//
#if GORGEOUS_ENGINE_VERSION < 500
template<typename T>
using TObjectPtr = T*;
#endif