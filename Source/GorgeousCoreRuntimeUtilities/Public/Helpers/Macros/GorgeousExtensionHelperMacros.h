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
 * Checks if the specified Gorgeous Plugin by checking for the existence of a predefined macro
 * 
 * @param PluginName The name of the plugin that should be checked for its existence.
 * @note Not used atm, planned to be introduced with upcoming new major version
 */
#define GORGEOUS_PLUGIN_INSTALLED(PluginName) (GORGEOUS_WITH_##PluginName)

/**
 * Checks if the specified Gorgeous General System is installed within the Core module.
 * 
 * @param SystemName The name of the system (e.g. InteractionFoundation) to check.
 */
#define GORGEOUS_GENERAL_SYSTEM_INSTALLED(SystemName) (GORGEOUS_SYSTEM_INSTALLED_##SystemName)