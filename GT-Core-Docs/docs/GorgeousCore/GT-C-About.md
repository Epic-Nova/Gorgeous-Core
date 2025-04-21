# 🔎 About Gorgeous Core 🔎

???+ info "Short Description"

    Gorgeous Core is a comprehensive Unreal Engine plugin that provides essential functionality for game development, including object variable management, quality-of-life features, runtime utilities, and editor enhancements to streamline your workflow and enhance your projects.

??? info "Long Description"

    Gorgeous Core is the foundation of the Gorgeous Things plugin ecosystem, designed to provide developers with powerful tools and utilities that simplify common development tasks. It offers a robust object variable system that allows for flexible data storage and management across different game components, from game instances to player controllers.

    The plugin is structured into four key modules:

    * **Core Runtime Module:**
        * Defines the base classes and interfaces for the Gorgeous Things ecosystem
        * Implements the object variable system for flexible data management
        * Provides conditional object chooser system for dynamic object selection
        * Supports functional structures with property change events

    * **Runtime Utilities Module:**
        * Offers essential helper functions for file and directory operations
        * Provides comprehensive logging capabilities with different importance levels
        * Includes templates for singleton pattern implementation
        * Contains utility macros for API endpoints and platform-specific code

    * **Editor Module:**
        * Extends the Unreal Editor with custom detail panels
        * Provides factory classes for creating Gorgeous objects
        * Implements functional structure customization
        * Supports detail customization for object variables

    * **Editor Utilities Module:**
        * Handles asset registration and management
        * Provides custom asset type actions
        * Implements factory classes for asset creation
        * Supports editor-specific utility functions

## 🚀 Key Features

### Object Variable System
* Flexible data storage and management across game components
* Type-safe variable handling with automatic serialization
* Support for single values, arrays, maps, and sets
* Integration with Unreal Engine's property system

### Enhanced Game Components
* Extended versions of core Unreal Engine classes
* Built-in object variable support
* Automatic property replication
* Editor integration for easy configuration

### Conditional Object Chooser System
* Dynamic object selection based on conditions
* Support for boolean, validity, and gameplay tag conditions
* Blueprint-friendly interface
* Editor tools for condition setup

### Editor Integration
* Custom detail panels for Gorgeous objects
* Factory classes for object creation
* Asset management tools
* Property customization support

### Comprehensive Logging
* Multiple importance levels (Information, Success, Warning, Error, Fatal)
* On-screen and output log support
* Configurable display duration
* Category-based filtering

### File and Directory Operations
* Simplified file management
* Directory handling utilities
* Platform-independent operations
* Error handling and validation

## 🛠️ Integration

To integrate Gorgeous Core into your project:

1. Add the plugin to your project's Plugins directory
2. Include the appropriate headers:

```cpp
// For Runtime features
#include "GorgeousCoreMinimalShared.h"
#include "GorgeousCoreRuntimeUtilitiesMinimalShared.h"

// For Editor features
#include "GorgeousCoreEditorMinimalShared.h"
#include "GorgeousCoreEditorUtilitiesMinimalShared.h"
```

## ⚙️ Configuration

Add the required modules to your project's Build.cs file:

```c#
// For Runtime features
PublicDependencyModuleNames.AddRange(new string[] { 
    "GorgeousCoreRuntime",
    "GorgeousCoreRuntimeUtilities"
});

// For Editor features
if (Target.Type == TargetType.Editor) {
    PrivateDependencyModuleNames.AddRange(new string[] {
        "GorgeousCoreEditor",
        "GorgeousCoreEditorUtilities"
    });
}
```

## 🤔 Frequently Asked Questions (FAQ)

**Q: What is the object variable system?**
A: The object variable system is a flexible data storage mechanism that allows you to attach named variables to various game components. It supports different variable types, provides type safety, automatic serialization, and network replication.

**Q: How do I access Gorgeous game components?**
A: You can access Gorgeous components using the standard Unreal Engine casting functions or through the provided global access functions. For example:
```cpp
if (auto* GorgeousGameInstance = Cast<UGorgeousGameInstance>(GetGameInstance()))
{
    // Work with the Gorgeous Game Instance
}
```

**Q: Can I use Gorgeous Core with Blueprints?**
A: Yes, most features are fully Blueprint-compatible. The object variable system, conditional object choosers, and logging functions are all exposed to Blueprints.

**Q: How do I create custom object variables?**
A: You can create custom object variables by:
1. Creating a new class that inherits from `UGorgeousObjectVariable`
2. Implementing the required functions
3. Using the provided factory in the editor or creating instances in code

**Q: How do I use the conditional object chooser system?**
A: The conditional object chooser system allows you to:
1. Create condition objects (Boolean, Validity, or Gameplay Tag)
2. Set up a conditional object chooser with your conditions
3. Configure the objects to be selected based on the conditions
4. Use the chooser to dynamically select objects at runtime

