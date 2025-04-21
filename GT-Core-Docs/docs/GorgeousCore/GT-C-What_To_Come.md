# 🔮 What's Coming to Gorgeous Core 🔮

???+ info "Short Description"

    Gorgeous Core is evolving with planned enhancements to its object variable system, functional structures, conditional object choosers, and build tools. These improvements will provide more flexible data management, better editor integration, and streamlined development workflows for Unreal Engine projects.

??? info "Long Description"

    Gorgeous Core is the foundation of the Gorgeous Things plugin ecosystem, designed to provide essential functionality that will be reused across all plugins in the family. This document outlines the planned features and improvements that will be implemented to consider the core "finished" (though it will continue to evolve with future projects to achieve "Battle Tested" status).

    The planned enhancements focus on four key areas:

    * **Object Variable System:** A highly dynamic system for addressing variables via pointer references, with improvements to customization, storage, and lifecycle management.
    * **Functional Structures:** Enhanced capabilities for standard Unreal Engine structures with property change callbacks.
    * **Conditional Object Choosers:** Flexible object selection based on conditions like gameplay tags and reference validity.
    * **Build System:** Tools to simplify module development and dependency management.

## 🎯 Planned Features

### Object Variable System Enhancements

The object variable system is a highly dynamic system that provides a way of addressing variables via pointer references. While this dynamism allows for adding new variable types, it also makes implementation challenging and requires deep interaction with the engine's source.

#### Custom Details Customization
* **Improved Default Value Appearance**
  * Creation of two dropdown menus and one FGuid as the variable identifier
  * Type dropdown: Lists every available object variable (C++ or Blueprint)
  * Count dropdown: Lists properties of the EObjectVariableCount_E enum
  * Warning messages for unimplemented count types
  * Dropdown menus only shown when necessary

#### Value Management
* **Default Value Construction**
  * Dynamic field construction based on Type and Count
  * Access to object variable defaults
  * Enum-based input type specification

#### Storage and Retrieval
* **Outer Access**
  * Save values in an extra trunk in the outer
  * Load values from the trunk via property name/signature
  * Enum selection for appropriate default value field

#### Initialization and Access
* **Lazy Initialization**
  * Initialize saved values from trunk when accessed
  * Use property name/signature as trunk identifier
  * Direct value return if already initialized

#### Advanced Features
* **Universal Access Functions**
  * Template-based Get and Set functions for C++
  * Type parsing via template arguments

* **Persistence**
  * Save object variables to disk via serialization
  * Support for runtime persistence

* **Hierarchical Structure**
  * Nesting: Parent-child relationships between variables
  * Visualization in a dedicated window (gorgeous.ov.list)

* **Lifecycle Management**
  * Persistent and non-persistent variables
  * Automatic cleanup of non-persistent variables after level switches
  * Persistence based on owner object lifecycle

> **Note:** This workflow will replace the structures currently representing default values, addressing memory allocation inefficiencies where the entire memory space is allocated even when only a single value is used.

### Functional Structures

Functional Structures enhance the capabilities of standard Unreal Engine structures by providing additional functionality:

* **Property Change Callbacks**
  * Structures with PostEditPropertyChange functionality
  * Callbacks when structure members are changed
  * Improved editor integration

> **Note:** This feature will be particularly useful for the dialogue system, allowing for dynamic asset selection based on property changes.

### Conditional Object Choosers

Conditional Object Choosers provide a flexible way to select objects based on conditions:

* **Condition-Based Selection**
  * Objects that return different object variables based on condition checks
  * Support for various condition types:
    * Gameplay Tag presence in containers
    * Reference validity checks
  * Branching logic based on condition results (Valid/Not Valid, Present/Not Present)

### Build System Improvements

The build system improvements aim to simplify module development:

* **Dependency Management**
  * Automatic scanning of source files for needed module dependencies
  * Automatic inclusion of dependencies in module names array
  * Dynamic link error resolution

* **Development Tools**
  * Internal Build.cs simplification tool
  * Internal Target.cs simplification tool
  * Support for third-party library integration

> **Note:** These improvements will be available not just for Gorgeous plugins but for any Unreal Engine project, providing a tool that simplifies coding quality while maintaining usability.

## 🛠️ Implementation Timeline

While specific release dates are not yet determined, the implementation will follow this general order:

1. **Phase 1: Core Functionality**
   * Basic object variable system enhancements
   * Initial functional structures implementation
   * Foundation for conditional object choosers

2. **Phase 2: Editor Integration**
   * Custom details customization
   * Improved property editing
   * Visualization tools

3. **Phase 3: Advanced Features**
   * Nesting and persistence
   * Universal access functions
   * Conditional object chooser implementation

4. **Phase 4: Build System**
   * Dependency management tools
   * Build.cs and Target.cs simplification
   * Third-party integration support

## 🤝 Contributing

We welcome contributions to help implement these features. If you're interested in contributing:

1. Check the [GitHub repository](https://github.com/Epic-Nova/Gorgeous-Core) for open issues
2. Review the [CONTRIBUTING.md](https://github.com/Epic-Nova/Gorgeous-Core/blob/master/.github/CONTRIBUTING.md) guidelines
3. Fork the repository and create a feature branch
4. Submit a pull request with your changes

## 🤔 Frequently Asked Questions (FAQ)

**Q: When will these features be available?**
A: The implementation will follow the phased approach outlined in the Implementation Timeline section. While specific release dates are not yet determined, we aim to prioritize features based on community feedback and development resources.

**Q: How can I stay updated on development progress?**
A: You can follow the [GitHub repository](https://github.com/Epic-Nova/Gorgeous-Core) for commit updates, check the [Issues page](https://github.com/Epic-Nova/Gorgeous-Core/issues) for tracking, and join our community discussions for the latest information.

**Q: Can I contribute to the development of these features?**
A: Yes! We welcome contributions from the community. You can help by implementing features, providing feedback, or reporting issues. See the Contributing section for more details.

**Q: Will these changes be backward compatible?**
A: We aim to maintain backward compatibility where possible. However, some enhancements may require updates to existing code. We will provide migration guides when necessary.

**Q: How can I provide feedback on these planned features?**
A: You can provide feedback by opening an issue on the [GitHub repository](https://github.com/Epic-Nova/Gorgeous-Core/issues), joining our community discussions, or contacting the development team directly.

---

*This roadmap is subject to change as development progresses and new requirements are identified.*