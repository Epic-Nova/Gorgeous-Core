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
#include "DataSchemaMapping/GorgeousDataSchemaMappingTypes.h"
#include "Helpers/GorgeousPropertyPathHelper.h"

bool FGorgeousDataSchemaSourceSnapshot_S::TryGetValueByPath(const FString& InPath, FString& OutValue) const
{
	for (const FGorgeousDataSchemaSourceValueEntry_S& Entry : ValueEntries)
	{
		if (GorgeousPropertyPath::ArePathStringsEquivalent(Entry.Path, InPath))
		{
			OutValue = Entry.ExportedValue;
			return true;
		}
	}

	return false;
}
