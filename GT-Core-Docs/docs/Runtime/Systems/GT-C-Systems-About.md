# 🧩 Blueprint Systems (Content/Systems)

???+ info "Short Description"

    Blueprint-first systems that ship as Content assets inside Gorgeous Core. These systems are ready-to-use building blocks (Blueprints, data assets, UI) for gameplay features such as playlists and team management.

??? info "Long Description"

    The systems in GorgeousCore/Content/Systems are Blueprint-centric feature packs. Each system provides:

    * Core Blueprint classes (actors, components, interfaces, function libraries).
    * Data definitions and structures (enums/structs/data containers).
    * User interface widgets and supporting textures.

    These systems are designed to be dropped into projects and customized without touching C++ code. They complement the runtime module by providing production-ready Blueprint workflows.

## 📦 Available Systems

* **Playlist System**
    * Orchestrates playlist execution and runtime state tracking.
    * Includes survey/data gathering UI widgets.
* **Team System**
    * Handles team definitions, team runtime variables, and UI presentation.
    * Provides Blueprint interfaces and helper libraries for team logic.

## 🧭 Asset Roots

* Playlist system assets live in: GorgeousCore/Content/Systems/Playlist
* Team system assets live in: GorgeousCore/Content/Systems/Team

## ✅ Recommended Usage

1. Duplicate a system’s assets into your project or plugin.
2. Configure data definitions and runtime variables.
3. Extend the Blueprint classes as needed for your game rules.
