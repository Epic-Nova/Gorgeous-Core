// Copyright (c) 2025 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2025 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                   Epic Nova is an independent entity,                     |
|         that has nothing in common with Epic Games in any capacity.       |
<==========================================================================*/

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

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

#endif