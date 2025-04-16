# 🧩 About this Module 🧩

???+ info "Short Description"

     Gorgeous Core is a versatile Unreal Engine plugin that provides essential runtime utilities for enhanced logging, world context access, singleton management, and various helper functions, streamlining development and promoting organized, efficient code.

??? info "Long Description"

    Gorgeous Core is designed to be a fundamental building block for Unreal Engine projects, offering a collection of essential tools and utilities that address common development challenges. This plugin simplifies complex tasks and promotes best practices, allowing developers to focus on creating compelling gameplay experiences.

    The Runtime Utilities module offers a collection of tools specifically designed to support the Gorgeous Things plugin family:

    * **General Utilities:**
        * Provides core functionality and interfaces for other Gorgeous Things plugins.
        * Defines enums and a precompiled header for efficient compilation.
    * **Logging:**
        * Offers a Blueprint library for versatile logging, enabling messages with different levels of importance and display options.
    * **Templates:**
        * Includes templates for creating UObjects with world context awareness and for implementing the singleton pattern.
    * **Helpers:**
        * Contains a set of helper functions for common tasks such as:
            * Converting file paths.
            * Working with directories and files.
            * Handling language localization.
            * String conversions.
        * Provides macros for:
            * Defining API endpoints.
            * Checking for plugin existence.
            * Platform-specific code.
            * Engine version compatibility.
    

## 🛠️ Integration

To integrate Gorgeous Core into your project, simply add the plugin to your project's Plugins directory.  Then, you can utilize the provided classes and functions in your C++ code. For example:

```cpp
// Example usage
#include "GorgeousCoreUtilitiesMinimalShared.h"
// ... your code ...
```

## ⚙️ Configuration (C++ only)

To configure Gorgeous Core, you need to add "GorgeousCoreRuntimeUtilities" to the PublicDependencyModuleNames/PrivateDependencyModuleNames array in your project's Build.cs file.  This ensures that your project can access the necessary runtime components.   

```c#
PublicDependencyModuleNames.AddRange(new string[] { "GorgeousCoreRuntimeUtilities" });
PrivateDependencyModuleNames.AddRange(new string[] { "GorgeousCoreRuntimeUtilities" });
```

## 🤔 Frequently Asked Questions (FAQ)

**Question:** What does the `GorgeousSingleton.h` template do?

**Answer:** It provides a convenient way to create singleton UObject classes, ensuring only one instance of the class exists. This is useful for manager classes or other objects that need to be globally accessible.


**Question:** How can I use the logging functionality?

**Answer:** The `GorgeousLoggingBlueprintFunctionLibrary.h` provides Blueprint Callable functions (e.g., `LogInformationMessage`, `LogWarningMessage`) to easily log messages with different levels of importance. Also the correct usage of this class is also documented here.
