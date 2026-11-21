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

// Default API host shared by the project settings and production URL constants.
#define GORGEOUS_API_DEFAULT_HOST "api.gorgeous.simsalabim.studio"

/**
 * Endpoint URL for the Gorgeous API.
 * All production requests require TLS and explicit user consent.
 */
#define GORGEOUS_API_ENDPOINT "https://" GORGEOUS_API_DEFAULT_HOST

/**
 * Endpoint URL for the Gorgeous API V1.
 */
#define GORGEOUS_API_V1_ENDPOINT GORGEOUS_API_ENDPOINT "/api/v1"

#define GORGEOUS_API_ENDPOINT_UPDATE_CHECK GORGEOUS_API_V1_ENDPOINT "/plugins/update-check"