// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"

// Layer Tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Gorgeous_UI_Layer_HUD, "UI.Layer.HUD");

// Input Actions without Natively Defined ones (Confirm, Cancel, NextTab, PreviousTab)
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Action_EscapeView, "UI.Action.EscapeView", 
	"Triggers the 'escape' action, which should close the current view or open a menu if on the main game layout.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Action_ListView, "UI.Action.ListView", 
	"Triggers the 'list' action, which should open a list view (e.g., inventory, quest log) if applicable.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Action_Left, "UI.Action.Left", 
	"Triggers the 'left' action, which should navigate left in a menu or carousel context.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Action_Right, "UI.Action.Right", 
	"Triggers the 'right' action, which should navigate right in a menu or carousel context.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Action_Up, "UI.Action.Up", 
	"Triggers the 'up' action, which should navigate up in a menu context.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Action_Down, "UI.Action.Down", 
	"Triggers the 'down' action, which should navigate down in a menu context.");

// Layout Signals
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Layout_RegisterLayer, "UI.Layout.RegisterLayer", 
	"Signal Bridge -> Send: Registers a widget stack as a named UI layer.")

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Layout_PushWidget, "UI.Layout.PushWidget",
	"Signal Bridge -> Send: Pushes a widget to a specific layer (async load + auto-theme).")


// Navigation Signals
UE_DEFINE_GAMEPLAY_TAG(TAG_Gorgeous_UI_Signal_Carousel_Next, "UI.Signal.Carousel.Next");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gorgeous_UI_Signal_Carousel_Prev, "UI.Signal.Carousel.Prev");

// System Signals
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Focus_Request, "UI.Focus.Request", 
	"Signal Bridge -> Send: Requests that a specific widget (by tag) takes focus.");

UE_DEFINE_GAMEPLAY_TAG(TAG_Gorgeous_UI_System_Message_Push, "UI.System.Message.Push");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gorgeous_UI_System_Message_Result, "UI.System.Message.Result");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Input_Action, "UI.Input.Action", 
	"Signal Bridge -> Send: Remotely triggers a button click or a back action.");
