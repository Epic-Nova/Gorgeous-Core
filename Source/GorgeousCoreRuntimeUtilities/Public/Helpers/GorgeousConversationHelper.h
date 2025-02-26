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

//<=============================--- Pragmas ---==============================>
#pragma once
//<-------------------------------------------------------------------------->

static FORCEINLINE FString GorgeousPathToRelativePath(FString FilePath)
{
	return "";
	//return FString(UGorgeousGlobals::GetGorgeousPlugin()->GetBaseDir() / FilePath);
}

/*
template <typename Enumeration>
static FORCEINLINE Enumeration GorgeousStringToEnum(const FString InValue)
{
	return static_cast<Enumeration>(StaticEnum<Enumeration>()->GetValueByNameString(InValue));
}

template <typename Enumeration>
static FORCEINLINE FString GorgeousEnumToString(const Enumeration InValue)
{
	return "";
	//return StaticEnum<Enumeration>()->GetNameStringByValue(static_cast<int64>(InValue));
}
*/