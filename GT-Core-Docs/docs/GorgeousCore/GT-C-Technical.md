# 🛠️ Technical 🛠️

???+ info "Short Description"

    Gorgeous Core is built on a modular architecture with four main components: Core Runtime, Runtime Utilities, Editor, and Editor Utilities. This technical documentation provides an overview of the architecture, class hierarchy, and key systems.

??? info "Long Description"

    The technical architecture of Gorgeous Core is designed to be modular, extensible, and efficient. It consists of four main modules that work together to provide a comprehensive set of tools and utilities for Unreal Engine development. This document explains the architecture, class hierarchy, and key systems in detail.

## 🏗️ Architecture

The Gorgeous Core plugin is structured into four main modules, split between core and utility modules:

### Core Modules
```mermaid
graph TD
    A[Gorgeous Core] --> B[Core Runtime Module]
    A --> D[Editor Module]
    
    B --> B1[Object Variables]
    B --> B2[Conditional Objects]
    B --> B3[Functional Structures]
    B --> B4[Base Classes]
    
    D --> D1[Detail Panels]
    D --> D2[Factory Classes]
    D --> D3[Property Customization]
    D --> D4[Editor Integration]
```

### Utility Modules
```mermaid
graph TD
    A[Gorgeous Core] --> C[Runtime Utilities Module]
    A --> E[Editor Utilities Module]
    
    C --> C1[File Helpers]
    C --> C2[Directory Helpers]
    C --> C3[Logging System]
    C --> C4[Templates]
    
    E --> E1[Asset Registration]
    E --> E2[Asset Type Actions]
    E --> E3[Asset Factories]
    E --> E4[Editor Utilities]
```

## 🔄 Module Structure

### Core Runtime Module
```mermaid
classDiagram
    class UGorgeousObjectVariable {
        +FName UniqueIdentifier
        +TMap~FName, UGorgeousObjectVariable*~ VariableRegistry
    }
    
    class UGorgeousConditionalObjectChooser {
        +TArray~UGorgeousCondition*~ Conditions
        +UGorgeousCondition* ConditionCheck
        +UGorgeousObjectVariable* DecideCondition()
    }
    
    class UGorgeousCondition {
        +uint8 CheckCondition()
    }
    
    UGorgeousConditionalObjectChooser <|-- UGorgeousCondition
    UGorgeousObjectVariable <|-- UGorgeousRootObjectVariable
    UGorgeousCondition <|-- UGorgeousBooleanCondition
    UGorgeousCondition <|-- UGorgeousIsValidCondition
    UGorgeousCondition <|-- UGorgeousGameplayTagCondition
```

### Editor Module
```mermaid
classDiagram
    class FGorgeousObjectVariableDetailCustomization {
    }
    
    class FGorgeousFunctionalStructureDetailCustomisation {
    }
    
    class UGorgeousFactory {
    }
    
    UFactory <|-- UGorgeousFactory
    UGorgeousFactory <|-- UGorgeousObjectVariableFactory
    UGorgeousFactory <|-- UGorgeousGameModeFactory
    UGorgeousFactory <|-- UGorgeousGameStateFactory
    UGorgeousFactory <|-- UGorgeousXXXFactory
```

### Editor Utilities Module
```mermaid
classDiagram
    class UGorgeousAssetRegistration_ES {
        +void RegisterNewAsset()
        +void RegisterNewCategory()
    }
    
    class FGorgeousAssetTypeAction {
    }
    
    class UGorgeousFactory {
        +virtual UObject* FactoryCreateNew() override
    }
    
    FAssetTypeActions_Base <|-- FGorgeousAssetTypeAction
    UFactory <|-- UGorgeousFactory
```

## 📝 Implementation Details

### Object Variable System
The object variable system uses a hierarchical structure:
```mermaid
graph TD
    A[Root Object Variable] --> B[Game Instance Variables]
    A --> C[Game Mode Variables]
    A --> D[Game State Variables]
    B --> B1[Player Variables]
    B --> B2[Level Variables]
    C --> C1[Gameplay Variables]
    D --> D1[Replicated Variables]
```

### Factory System
The factory system for creating Gorgeous objects:
```mermaid
graph LR
    A[Editor Module] --> B[Factory Base Class]
    B --> C[Object Variable Factory]
    B --> D[Game Component Factories]
    B --> E[Condition Factories]
    
    D --> D1[Game Mode Factory]
    D --> D2[Game State Factory]
    D --> D3[Player Controller Factory]
    
    E --> E1[Boolean Condition Factory]
    E --> E2[Is Valid Condition Factory]
    E --> E3[Gameplay Tag Factory]
```

### Asset Registration
The asset registration system:
```mermaid
sequenceDiagram
    participant Editor as Editor Module
    participant Registry as Asset Registry
    participant Factory as Asset Factory
    
    Editor->>Registry: Register Asset Type
    Registry->>Factory: Create Factory Instance
    Factory->>Registry: Register Factory
    Registry->>Editor: Confirm Registration
```

## 🔧 Build System

The module dependencies are managed through Build.cs files:

```mermaid
graph TD
    A[Project] --> B[GorgeousCoreRuntime]
    A --> C[GorgeousCoreRuntimeUtilities]
    A --> D[GorgeousCoreEditor]
    A --> E[GorgeousCoreEditorUtilities]
    
    B --> F[Core]
    B --> G[CoreUObject]
    
    C --> B
    C --> H[Engine]
    
    D --> B
    D --> C
    D --> I[UnrealEd]
    
    E --> D
    E --> J[AssetTools]
```

## 🔍 Key Classes and Interfaces

### Core Runtime
- `UGorgeousObjectVariable`: Base class for all object variables
- `UGorgeousConditionalObjectChooser`: Handles dynamic object selection
- `UGorgeousCondition`: Base class for all conditions

### Editor
- `FGorgeousObjectVariableDetailCustomization`: Customizes object variable properties
- `FGorgeousFunctionalStructureDetailCustomisation`: Handles structure property changes
- `UGorgeousFactory`: Base class for all Gorgeous object factories

### Editor Utilities
- `FGorgeousAssetTypeAction`: Defines how assets behave in the editor
- `FGorgeousAssetRegistration`: Handles asset registration and unregistration
- `UGorgeousFactory`: Creates new Gorgeous assets

## 🔄 Extension Points

1. **Custom Object Variables**
```cpp
UCLASS()
class MYGAME_API UMyCustomVariable : public UGorgeousObjectVariable
{
    GENERATED_BODY()
    // Implementation
};
```

2. **Custom Conditions**
```cpp
UCLASS()
class MYGAME_API UMyCustomCondition : public UGorgeousCondition
{
    GENERATED_BODY()
    // Implementation
};
```

3. **Custom Asset Types**
```cpp
class FMyCustomAssetTypeAction : public FGorgeousAssetTypeAction
{
    // Implementation
};
```

## 🔧 Configuration Options

1. **Build Configuration**

```c#
public class MyGameModule : ModuleRules
{
    public MyGameModule(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] {
            "GorgeousCoreRuntime",
            "GorgeousCoreRuntimeUtilities"
        });
        
        if (Target.Type == TargetType.Editor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] {
                "GorgeousCoreEditor",
                "GorgeousCoreEditorUtilities"
            });
        }
    }
}
```