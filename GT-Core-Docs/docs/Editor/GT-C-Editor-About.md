# 🧩 About this Module 🧩

???+ info "Short Description"

    Gorgeous Core is a versatile Unreal Engine plugin that enhances the Unreal Engine editor by providing tools for streamlined structure editing.

??? info "Long Description"

    Gorgeous Core Editor is designed to be a fundamental building block for Unreal Engine projects, offering a collection of essential tools and utilities that address common development challenges. This plugin simplifies complex tasks and promotes best practices, allowing developers to focus on creating compelling gameplay experiences.

    The **Editor** module focuses on enhancing the editor experience:

    * **Functional Structures:**
        * Provides a subsystem for registering functional structures, enabling them to receive `PostEditChangeProperty` and `PreEditChangeProperty` events. This gives developers tighter control over how structure properties are edited within the Unreal Engine editor.
    

## 🛠️ Integration

To integrate Gorgeous Core Editor into your project, simply add the plugin to your project's Plugins directory.  Then, you can utilize the provided classes and functions in your C++ code. For example:

```cpp
// Example usage
#include "GorgeousCoreEditorMinimalShared.h"
// ... your code ...
```

## ⚙️ Configuration (C++ only)

To configure Gorgeous Core, you need to add "GorgeousCoreEditor" to the PublicDependencyModuleNames/PrivateDependencyModuleNames array in your project's Build.cs file. This ensures that your project can access the necessary editor components.

```c#
PublicDependencyModuleNames.AddRange(new string[] { "GorgeousCoreEditor" });
PrivateDependencyModuleNames.AddRange(new string[] { "GorgeousCoreEditor" });
```

## 🤔 Frequently Asked Questions (FAQ)

**Question:** What are Functional Structures in the Editor module?

**Answer:** Functional Structures are a feature that allows developers to receive callbacks when properties of a structure are changed in the Unreal Engine editor. This enables real-time updates and custom behavior when editing structure properties.


**Question:** Why would I use the Editor module?

**Answer:** The Editor module enhances the Unreal Engine editor, providing tools that can streamline content creation workflows and give developers more control over how data is edited and displayed.
