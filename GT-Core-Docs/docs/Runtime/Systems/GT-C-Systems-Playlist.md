# 🎶 Playlist System (Blueprint)

???+ info "Short Description"

    The Playlist system provides a Blueprint-driven flow for managing playlist state, execution, and player-facing data gathering UI.

??? info "Long Description"

    The Playlist system is a content-first feature pack that combines runtime state, data definitions, and UI presentation to drive playlists. It ships with executor/handler Blueprints, object-variable containers, and a data gathering UI framework.

## 🧱 Core Blueprints

* **BP_PlaylistExecutor_A** – Executes playlist flow and state transitions.
* **BP_PlaylistHandling_AC** – Actor component that coordinates playlist logic.
* **BP_PlaylistObject** – Core playlist UObject.
* **BP_Playlist_BFL** – Blueprint function library helpers.
* **BP_Playlist_I** – Interface for playlist-aware actors/objects.
* **BP_PlaylistUserInterface_I** – Interface for UI integration.

## 📊 Runtime Data

* **BP_PlaylistRuntime_OV** – Runtime object variable container.
* **BP_PlaylistClientRuntime_OV** – Client-specific runtime state.
* **BP_PlaylistDataContainer_OV** – Data container for playlist definitions.

## 🧾 Data Definitions & Structures

* **PlaylistState_E** – Playlist state enum.
* **DataGeatheringSchema_S** – Survey schema definition.
* **DataGeatheringProperty_S** – Survey property descriptor.
* **DataGeatheringOptInData_S** – Opt-in data definition.
* **EliagbleTeam_S** – Team eligibility structure.

## 🖥️ User Interface Widgets

* **BP_Countdown_CAW** – Countdown overlay.
* **BP_PlaylistOverlayBase_CAW** – Base overlay widget.
* **BP_PlaylistSurvey_CUW** – Survey UI widget.
* **BP_PlaylistDataGeather_CAW** – Data gathering wrapper.
* **BP_PlaylistDataGeatherPropertyBase_CUW** – Base property widget.
* **BP_PlaylistDataGeatherBoolean_CUW** – Boolean input.
* **BP_PlaylistDataGeatherButton_CUW** – Button input.
* **BP_PlaylistDataGeatherCheckbox_CUW** – Checkbox input.

## 🔁 Typical Flow

1. **BP_PlaylistHandling_AC** initializes the playlist object.
2. **BP_PlaylistExecutor_A** transitions playlist state via **PlaylistState_E**.
3. **Runtime OV containers** track live data.
4. **Survey UI** gathers input based on **DataGeatheringSchema_S**.

## ✅ Notes

* Asset paths: GorgeousCore/Content/Systems/Playlist
* The data gathering subfolder is intentionally named “DataGeathering” to match assets.
