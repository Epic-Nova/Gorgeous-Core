# ✨ GorgeousCoreRuntime ✨

[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen.svg)](https://example.com/build)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

> **About**: GorgeousCoreRuntime is the core module of the *Gorgeous Things* plugin, providing essential runtime components and functionality. It is responsible for loading the core runtime components and managing referenced third-party libraries.

## 🚀 Features

### `GorgeousCoreMinimalShared.h`
> **Description**: This file defines minimal shared components for the *GorgeousCoreRuntime* module. It ensures that only module-defined classes are included and prevents circular dependencies.

#### **Important Notes:**
- Only include classes that belong to this module.
- Do **not** use this file within the *GorgeousCoreRuntime* module itself to avoid circular dependencies.

#### **Includes:**
- **Global Runtime Configuration:** `GorgeousCoreRuntimeGlobals.h`
- **Object Variables:**
  - `GorgeousObjectVariable.h`
  - `GorgeousObjectVariableEnums.h`
  - `GorgeousRootObjectVariable.h`
  - `GorgeousObjectVariableDefinitions.h`
  - Helper Macros: `GorgeousObjectVariableHelperMacros.h`
- **Conditional Object Choosers:**
  - `GorgeousConditionalObjectChooser.h`
  - `GorgeousConditionalObjectChooserEnums.h`
  - `GorgeousConditionalObjectChooserStructures.h`
  - Conditions:
    - `GorgeousCondition.h`
    - `GorgeousBooleanCondition.h`
    - `GorgeousGameplayTagCondition.h`
    - `GorgeousIsValidCondition.h`
- **Functional Structures:** `GorgeousFunctionalStructure.h`
- **Interfaces for Object Variables:**
  - `GorgeousObjectVariableInteraction_I.h`
  - `GorgeousSingleObjectVariablesGetter_I.h`
  - `GorgeousSingleObjectVariablesSetter_I.h`
  - `GorgeousArrayObjectVariablesGetter_I.h`
  - `GorgeousArrayObjectVariablesSetter_I.h`
  - `GorgeousMapObjectVariablesGetter_I.h`
  - `GorgeousMapObjectVariablesSetter_I.h`
  - `GorgeousSetObjectVariablesGetter_I.h`
  - `GorgeousSetObjectVariablesSetter_I.h`

### `FGorgeousCoreRuntimeModule`
> **Description**: The core module responsible for managing runtime components and dependencies.

#### **Key Responsibilities:**
- Initializes and shuts down the core runtime.
- Manages dependencies and third-party libraries.

#### **Methods:**
```cpp
/**
 * The override function of the module interface that is called to load up this module.
 */
virtual void StartupModule() override;

/**
 * The override function of the module interface that is called to unload this module.
 */
virtual void ShutdownModule() override;
```

## 🛠️ Installation

To use *GorgeousCoreRuntime* in your project, include the necessary headers and ensure all dependencies are properly configured.

```cpp
#include "GorgeousCoreMinimalShared.h"
```

## ⚙️ Configuration

No special configuration is required. Ensure all dependencies are correctly linked.

## 📚 Usage Examples

```cpp
// Example: Using a GorgeousObjectVariable
GorgeousObjectVariable MyVar;
MyVar.SetValue(SomeValue);
UE_LOG(LogTemp, Warning, TEXT("Value: %s"), *MyVar.ToString());
```

## 🔗 Blueprint Integration (Optional)

If the module provides Blueprint functionality, describe how to use it here.

## 🤔 FAQ

**Q:** What is the purpose of *GorgeousCoreMinimalShared.h*?
**A:** It provides shared definitions while preventing circular dependencies in *GorgeousCoreRuntime*.

**Q:** Can I modify *GorgeousCoreMinimalShared.h*?
**A:** Only if you follow the guidelines to prevent dependency issues.

## 📄 License

This project is licensed under the **MIT License**. See [LICENSE](LICENSE) for details.

## 🙏 Acknowledgments

Special thanks to *Simsalabim Studios* and *Epic Nova* for their contributions to this project.

