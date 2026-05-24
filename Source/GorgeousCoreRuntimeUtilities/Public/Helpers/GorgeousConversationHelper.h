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
//<--------------------------=== Engine Includes ===------------------------->
#include "Misc/EngineVersionComparison.h"
#include "Helpers/GorgeousPathHelper.h"
//<-------------------------------------------------------------------------->

/**
 * Returns an enum index from the enum value as the string.
 * 
 * @tparam Enumeration the enum that should be converted
 * @param InValue The string value that should be converted to a enum index
 * @return The string value
 */
template <typename Enumeration>
static FORCEINLINE Enumeration GorgeousStringToEnum(const FString InValue)
{
	return static_cast<Enumeration>(StaticEnum<Enumeration>()->GetValueByNameString(InValue));
}

/**
 * Returns the specified enum value as a string.
 * 
 * @tparam Enumeration the enum that should be converted
 * @param InValue The value of the enum that should be converted
 * @return The enum value as a string
 */
template <typename Enumeration>
static FORCEINLINE FString GorgeousEnumToString(const Enumeration InValue)
{
	return StaticEnum<Enumeration>()->GetNameStringByValue(static_cast<int64>(InValue));
}