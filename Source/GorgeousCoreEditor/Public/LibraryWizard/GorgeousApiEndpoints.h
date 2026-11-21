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
#include "Containers/UnrealString.h"
//<-------------------------------------------------------------------------->

namespace GorgeousApiEndpoints
{
    /**
     * Resolves a path using the current API Origin and Force Dev Mode settings.
     *
     * @param Path An optional path under the API origin, including its leading slash.
     * @return The full URL for the selected origin.
     */
    GORGEOUSCOREEDITOR_API FString GetEndpoint(const FString& Path = FString());

    /**
     * Resolves a route under the versioned API.
     *
     * @param Path The route path including its leading slash, such as /news.
     * @return The full URL under /api/v1.
     */
    GORGEOUSCOREEDITOR_API FString GetV1Endpoint(const FString& Path);

    /**
     * Checks the transport policy for an incoming URL.
     *
     * @param URL The URL supplied by the API or the final URL of a response.
     * @param bAllowDeveloperOrigin Whether this operation can use the developer HTTP origin.
     * @return Whether this URL uses HTTPS or the explicitly enabled developer origin.
     */
    GORGEOUSCOREEDITOR_API bool IsAllowedURL(const FString& URL, bool bAllowDeveloperOrigin = true);
}