# 🧩 About this Module 🧩

???+ info "Short Description"

    The Gorgeous Core Editor Utilities module extends the Unreal Editor with custom asset management, factory classes, and utility functions that streamline the creation and management of Gorgeous objects, providing a seamless development experience for plugin users.

??? info "Long Description"

    The Editor Utilities module is a crucial component of the Gorgeous Core plugin, designed to enhance the Unreal Editor experience for developers working with Gorgeous objects. This module provides specialized tools and utilities that simplify asset creation, management, and customization within the editor environment.

    The Editor Utilities module offers a collection of tools specifically designed to support the Gorgeous Things plugin family:

    * **Asset Management:**
        * Provides custom asset type actions for Gorgeous objects
        * Implements asset registration and management systems
        * Offers specialized asset creation workflows
    * **Factory Classes:**
        * Includes factory classes for creating various Gorgeous objects
        * Supports custom object creation with appropriate defaults
        * Provides specialized editors for complex object types
    * **Editor Integration:**
        * Extends the editor with custom detail panels
        * Implements property customization for Gorgeous objects
        * Provides specialized visualization tools
    * **Utility Functions:**
        * Contains helper functions for editor-specific tasks
        * Offers tools for asset validation and verification
        * Provides utilities for editor automation

## 🛠️ Integration

To integrate the Editor Utilities module into your project, add the plugin to your project's Plugins directory. Then, you can utilize the provided classes and functions in your C++ code. For example:

```cpp
// Example usage
#include "GorgeousCoreEditorUtilitiesMinimalShared.h"
// ... your code ...
```

## ⚙️ Configuration (C++ only)

To configure the Editor Utilities module, you need to add "GorgeousCoreEditorUtilities" to the PrivateDependencyModuleNames array in your project's Build.cs file. This ensures that your project can access the necessary editor components.

```c#
if (Target.Type == TargetType.Editor) {
    PrivateDependencyModuleNames.AddRange(new string[] { "GorgeousCoreEditorUtilities" });
}
```

## 🤔 Frequently Asked Questions (FAQ)

**Question:** What are asset type actions and how do they benefit my workflow?

**Answer:** Asset type actions define how assets appear and behave in the editor. The Editor Utilities module provides custom asset type actions for Gorgeous objects, allowing for specialized context menus, thumbnails, and creation workflows that streamline your development process.

**Question:** How can I create custom factory classes for my Gorgeous objects?

**Answer:** You can create custom factory classes by inheriting from the base factory classes provided by the Editor Utilities module. These factory classes handle the creation of objects with appropriate defaults and provide specialized editors for complex object types.

**Question:** How do I implement custom detail panels for my Gorgeous objects?

**Answer:** The Editor Utilities module provides a framework for creating custom detail panels. You can extend the base detail customization classes to create specialized editors for your Gorgeous objects, allowing for intuitive property editing and visualization.