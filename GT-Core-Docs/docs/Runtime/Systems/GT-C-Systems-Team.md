# 👥 Team System (Blueprint)

???+ info "Short Description"

    The Team system provides Blueprint utilities, data definitions, and UI widgets for team management, team state, and presentation.

??? info "Long Description"

    The Team system is a Blueprint-first toolkit for defining teams, managing runtime state, and presenting team UI. It integrates with object variables, conditional object choosers, and provides a ready-to-extend set of assets.

## 🧱 Core Blueprints

* **BP_TeamManager_AC** – Team manager component.
* **BP_TeamInteractor_AC** – Interaction entry point for team changes.
* **BP_TeamPlayerModifier_A** – Actor for modifying player team data.
* **BP_Team_BFL** – Team helper library.
* **BP_Team_I** – Team interface.
* **BP_Team_COC** – Conditional object chooser for team logic.
* **BP_Team_C** – Team definition asset.
* **BP_Team_OV** – Team object variable.
* **BP_TeamRuntime_OV** – Runtime team state container.

## 📚 Data Definitions

* **TeamData_S** – Team metadata struct.
* **MemberData_S** – Member data struct.

## 🖥️ User Interface

* **BP_TeamCard_CUW** – Team card widget.

## 🎨 Textures

* **T_TeamHeroPlaceholder** – Placeholder hero texture.
* **T_TeamIconPlaceholder** – Placeholder icon texture.
* **T_TeamMembers** – Team members texture.

## ✅ Notes

* Asset paths: GorgeousCore/Content/Systems/Team
