#pragma once
#include "CoreMinimal.h"

/** Numeric minimum-version comparison, accepting descriptor prefixes/suffixes. */
namespace GorgeousVersionComparison
{
    inline bool Parse(const FString& Version, int32 (&Parts)[3])
    {
        int32 Cursor = 0;
        while (Cursor < Version.Len() && !FChar::IsDigit(Version[Cursor])) ++Cursor;
        if (Cursor == Version.Len()) return false;
        for (int32 Index = 0; Index < 3; ++Index)
        {
            const int32 Start = Cursor;
            while (Cursor < Version.Len() && FChar::IsDigit(Version[Cursor])) ++Cursor;
            if (Cursor - Start > 9) return false;
            Parts[Index] = Cursor > Start ? FCString::Atoi(*Version.Mid(Start, Cursor - Start)) : 0;
            if (Cursor >= Version.Len() || Version[Cursor] != '.') break;
            ++Cursor;
        }
        return true;
    }

    inline bool IsOlder(const FString& Current, const FString& Required)
    {
        int32 Installed[3] = {}, Minimum[3] = {};
        if (!Parse(Current, Installed) || !Parse(Required, Minimum)) return false;
        for (int32 Index = 0; Index < 3; ++Index)
            if (Installed[Index] != Minimum[Index]) return Installed[Index] < Minimum[Index];
        return false;
    }
}
