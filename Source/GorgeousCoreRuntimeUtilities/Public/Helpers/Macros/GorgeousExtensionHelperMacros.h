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

/**
 * Checks if the specified Gorgeous Plugin by checking for the existence of a predefined macro
 * 
 * @param PluginName The name of the plugin that should be checked for its existence.
 */
#define GORGEOUS_PLUGIN_INSTALLED(PluginName) (PluginName##_INSTALLED == 1)