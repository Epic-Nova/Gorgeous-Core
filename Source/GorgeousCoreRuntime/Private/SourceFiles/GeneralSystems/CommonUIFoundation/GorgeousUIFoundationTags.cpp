// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationTags.h"

// System Signals
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Focus_Request, "GT.UI.Action.Focus.Request", 
	"Signal Bridge -> Send: Requests that a specific widget (by tag) takes focus.");

UE_DEFINE_GAMEPLAY_TAG(TAG_Gorgeous_UI_System_Message_Push, "GT.UI.System.Message.Push");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gorgeous_UI_System_Message_Result, "GT.UI.System.Message.Result");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Input_Action, "GT.UI.Action.Signal.Input", 
	"Signal Bridge -> Send: Remotely triggers a button click or a back action.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Context_Primary, "GT.UI.Context.Primary", "Representing an active Input Consumer Context (Used for Conflict Detection Internally)")

// Layer Tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Gorgeous_UI_Layer_HUD, "GT.UI.Layer.HUD");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Layer_ActionBar, "GT.UI.Layer.HUD.ActionBar", 
	"Dedicated layer for the action bar. This layer is pushed to the primary layout by default in AGorgeousHUD, but can be used by any widget stack that needs to render above the main game view.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Layer_Message, "GT.UI.Layer.HUD.Message",
	"Dedicated layer for transient messages (e.g. combat text, notifications, toasts, banners, modal or badge popups). This layer should be used for widgets that render temporary messages to the player, to ensure they appear above all other layers.");


UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Layer_TabList, "GT.UI.Layer.TabList", 
	"Dedicated layer for tab lists. This layer should be used for widgets that render tab buttons, to ensure they receive input before the content layers.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Layer_EscapeView, "GT.UI.Layer.EscapeView", 
	"Dedicated layer for escape views (e.g. pause menu, settings). This layer should be used for widgets that render full-screen menus, to ensure they receive input before all other layers.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Layer_ListView, "GT.UI.Layer.ListView", 
	"Dedicated layer for list views (e.g. inventory, quest log). This layer should be used for widgets that render scrollable lists, to ensure they receive input before the main game view but after critical UI like the action bar and tab lists.");


// Input Actions without Natively Defined ones (Confirm, Cancel, NextTab, PreviousTab)
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Action_EscapeView, "UI.Action.EscapeView", 
	"Triggers the 'escape' action, which should close the current view or open a menu if on the main game layout.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Action_ListView, "UI.Action.ListView", 
	"Triggers the 'list' action, which should open a list view (e.g., inventory, quest log) if applicable.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Action_Confirm, "UI.Action.Confirm", 
	"Triggers the 'confirm' action, which should activate the currently focused button or UI element.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Action_Cancel, "UI.Action.Cancel", 
	"Triggers the 'cancel' action, which should close the current menu or go back in navigation.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Action_PreviousTab, "UI.Action.PreviousTab", 
	"Triggers the 'previous tab' action, which should navigate to the previous tab in a tabbed interface.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Action_NextTab, "UI.Action.NextTab", 
	"Triggers the 'next tab' action, which should navigate to the next tab in a tabbed interface.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Action_Left, "UI.Action.Left", 
	"Triggers the 'left' action, which should navigate left in a menu or carousel context.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Action_Right, "UI.Action.Right", 
	"Triggers the 'right' action, which should navigate right in a menu or carousel context.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Action_Up, "UI.Action.Up", 
	"Triggers the 'up' action, which should navigate up in a menu context.");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Action_Down, "UI.Action.Down", 
	"Triggers the 'down' action, which should navigate down in a menu context.");

// Layout Signals
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Layout_RegisterLayer, "GT.UI.Layer.Signal.Layout.RegisterLayer", 
	"Signal Bridge -> Send: Registers a widget stack as a named UI layer.")

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gorgeous_UI_Layout_PushWidget, "GT.UI.Layer.Signal.Layout.PushWidget",
	"Signal Bridge -> Send: Pushes a widget to a specific layer (async load + auto-theme).")


// Navigation Signals
UE_DEFINE_GAMEPLAY_TAG(TAG_Gorgeous_UI_Signal_Carousel_Next, "UI.Action.Signal.Carousel.Next");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gorgeous_UI_Signal_Carousel_Prev, "UI.Action.Signal.Carousel.Prev");


