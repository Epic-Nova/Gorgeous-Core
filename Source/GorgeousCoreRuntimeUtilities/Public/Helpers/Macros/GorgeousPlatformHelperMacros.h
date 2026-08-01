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

#if PLATFORM_WINDOWS

/**
 * Allows the compilation of the code inside This when the platform is Windows, for every other platform Other is used.
 * 
 * @param This The code for when the platform is Windows
 * @param Other The code for when the platform is anything except Windows
 */
#define GORGEOUS_WINDOWS(This, Other) This

/**
 * Allows the compilation of the code inside this macro only if the current platform is Windows.
 */
#define GORGEOUS_WINDOWS_ONLY(...) __VA_ARGS__

#else
#define GORGEOUS_WINDOWS(This, Other) Other
#define GORGEOUS_WINDOWS_ONLY(...)
#endif