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

/**
 * Endpoint URL for the Gorgeous API.
 * @note Not used atm, planned to be introduced with upcoming new major version
 * @note For not http as we are in dev @todo: Smart switching based on the force http mdoe in the library config
 */
#define GORGEOUS_API_ENDPOINT "http://api.gorgeous.simsalabim.studio"

/**
 * Endpoint URL for the Gorgeous API V1.
 */
#define GORGEOUS_API_V1_ENDPOINT GORGEOUS_API_ENDPOINT "/api/v1"

#define GORGEOUS_API_ENDPOINT_UPDATE_CHECK GORGEOUS_API_V1_ENDPOINT "/systems/update-check"