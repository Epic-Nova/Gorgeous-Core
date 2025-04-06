# 🧩 About this Module 🧩

???+ info "Short Description"

    Gorgeous Core is a versatile Unreal Engine plugin that provides essential runtime tools for object variable management, conditional logic, and structure enhancements, simplifying development and promoting organized, efficient code.

??? info "Long Description"

    Gorgeous Core is designed to be a fundamental building block for Unreal Engine projects, offering a collection of essential tools and utilities that address common development challenges. This plugin simplifies complex tasks and promotes best practices, allowing developers to focus on creating compelling gameplay experiences.

    The Runtime module offers a core collection of utilities intended to support the operational needs of your game during execution:

    * **General:**
        * Provides core runtime functionality for other modules. 
        * Defines UObject classes that are widely used by interface classes in other Gorgeous plugins.
    * **Object Variables:**
        * Defines a system for creating and managing variables as objects, including enums, a registry for level switches, and command definitions. 
        * Provides base classes and definitions for various types of object variables (single, array, map, set).
        * Includes interfaces for getting and setting object variable values.
        * Offers helper macros to simplify object variable definitions. 
    * **Conditional Object Chooser:**
        * Defines a system for selecting object variables based on conditions, including enums and structures. 
        * Provides classes for conditional object selection and various types of conditions (boolean, validity, gameplay tag-based). 
    * **Functional Structures:**
        * Enables structures to receive `PostEditChangeProperty` and `PreEditChangeProperty` events, allowing for more control over property changes in the editor.
    

## 🛠️ Integration

To integrate Gorgeous Core into your project, simply add the plugin to your project's Plugins directory.  Then, you can utilize the provided classes and functions in your C++ code. For example:

```cpp
// Example usage
#include "GorgeousCoreMinimalShared.h"
// ... your code ...
```

## ⚙️ Configuration (C++ only)

To configure Gorgeous Core, you need to add "GorgeousCoreRuntime" to the PublicDependencyModuleNames/PrivateDependencyModuleNames array in your project's Build.cs file.  This ensures that your project can access the necessary runtime components.   

```c#
PublicDependencyModuleNames.AddRange(new string[] { "GorgeousCoreRuntime" });
PrivateDependencyModuleNames.AddRange(new string[] { "GorgeousCoreRuntime" });
```

## 🤔 Frequently Asked Questions (FAQ)

**Question:** What is the purpose of the GorgeousCoreMinimalShared.h header file?

**Answer:** It provides shared definitions for the GorgeousCoreRuntime module while preventing circular dependency issues.  It should only include classes that belong to the GorgeousCoreRuntime module and should not be included within the GorgeousCoreRuntime module itself.


**Question:** How do Object Variables work in Gorgeous Core?

**Answer:** Gorgeous Core's Object Variable system allows you to define variables as UObjects, providing a flexible and extensible way to store and manipulate data.  This system supports various variable types (single, array, map, set) and includes features like network replication, getter/setter interfaces, and dynamic property management. 
