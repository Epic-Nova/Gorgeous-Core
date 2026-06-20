// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#pragma once

#include "NativeGameplayTags.h"

//@TODO

/**
 * System-defined gameplay tags for the Gorgeous UI Foundation.
 */

// System Signals
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Focus_Request);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Focus_Changed);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Focus_Navigated);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_System_Message_Push);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_System_Message_Result);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Input_Action);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Context_Primary);

// Layer Tags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Layer_HUD);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Layer_ActionBar);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Layer_Message);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Layer_TabList);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Layer_EscapeView);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Layer_ListView);

// Input Actions without Natively Defined ones (Confirm, Cancel, NextTab, PreviousTab)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Action_EscapeView)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Action_ListView)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Action_Confirm)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Action_Cancel)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Action_PreviousTab)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Action_NextTab)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Action_Left)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Action_Right)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Action_Up)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Action_Down)

// Layout Signals
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Layout_RegisterLayer);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Layout_PushWidget);

// Navigation Signals
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Signal_Carousel_Next);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Signal_Carousel_Prev);


// Layout & Layer Signals
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Layout_PushWidget);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gorgeous_UI_Layout_RegisterLayer);
