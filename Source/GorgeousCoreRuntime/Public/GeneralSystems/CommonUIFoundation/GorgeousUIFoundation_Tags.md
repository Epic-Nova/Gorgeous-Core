# 📡 Gorgeous UI Foundation: Signal & Tag Registry

This document lists all the "Reserved" Gameplay Tags used by the system for communication between systems, layouts, and widgets.

---

## 🛠️ Commands (Signals you SEND)
Dispatch these signals to trigger system-wide actions.

| Tag | Payload Type | Description |
| :--- | :--- | :--- |
| `UI.Layout.PushWidget` | `FGorgeousUIMessageRequest` | Pushes a widget to a specific layer (async load + auto-theme). |
| `UI.Layout.RegisterLayer` | `FGorgeousRegisterLayerPayload` | Registers a widget stack as a named UI layer. |
| `UI.Message.Request` | `FGorgeousUIMessageRequest` | Triggers a confirmation dialog/message box flow. |
| `UI.Focus.Request` | `FGorgeousFocusRequestPayload` | Requests that a specific widget (by tag) takes focus. |
| `UI.Input.Action` | `FGorgeousInputActionPayload` | Remotely triggers a button click or a back action. |

---

## 🔔 Events (Signals you LISTEN to)
The system dispatches these when things happen.

| Tag | Payload Type | Description |
| :--- | :--- | :--- |
| `UI.Layout.Transitioning` | `FGorgeousProgressUpdatePayload` | Fired when any layer is sliding/fading (1.0 = started, 0.0 = finished). |
| `UI.Theme.Applied` | `UGorgeousUITheme_DA*` | Fired when a new aesthetic is applied (Widgets use this to re-skin). |
| `UI.State.Switched` | `UCommonUIState_DA*` | Fired when the global UI context (Combat, Menu) changes. |

---

## 🎯 Binding Tags (Widget IDs)
Use these tags inside the **BindingTag** property of your widgets to address them directly.

*   **Pattern**: `UI.[Feature].[ElementName]`
*   **Examples**:
    *   `UI.Inventory.Grid`
    *   `UI.Player.HealthBar`
    *   `UI.Menu.StartButton`
    *   `UI.Menu.Tabs` (For `UGorgeousCommonTabList`)

---

## 🎮 Action Tags (Icon Swapping)
Use these in the **ActionTag** property to enable dynamic hardware icon swapping.

*   `UI.Action.Confirm`
*   `UI.Action.Cancel`
*   `UI.Action.Back`
*   `UI.Action.Interact`

---

## 🔊 Audio Tags (Themed Sounds)
Use these in the **HoverSoundTag** and **ClickSoundTag** of Gorgeous buttons.

*   `UI.Audio.Hover`
*   `UI.Audio.Click`
*   `UI.Audio.Confirm`
*   `UI.Audio.Error`

---

### "Decoupled. Reflected. Gorgeous."
