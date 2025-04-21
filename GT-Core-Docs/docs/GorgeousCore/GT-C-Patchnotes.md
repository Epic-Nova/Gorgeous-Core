# 🆕 Patchnotes 🆕

## Gorgeous Core v0.8

### 🚀 New Features

* **Quality of Life Features**
  * Added enhanced game components with object variable capabilities
  * Implemented persistent data storage across game sessions
  * Added support for game instance, game mode, game state, player controller, player state, and world settings extensions

* **Documentation Improvements**
  * Complete overhaul of documentation system
  * Added comprehensive README.md with detailed plugin information
  * Created detailed technical documentation with architecture diagrams
  * Implemented MkDocs-based documentation site with custom domain

### 🛠️ Improvements

* **Build System**
  * Fixed project packaging issues
  * Added platform blacklist for better compatibility
  * Improved module dependency management

* **Editor Integration**
  * Enhanced asset registration system
  * Improved factory classes for object creation
  * Added custom detail panels for better property editing

* **Runtime Utilities**
  * Enhanced file and directory operations
  * Improved logging system with multiple importance levels
  * Added templates for singleton pattern implementation

### 🐛 Bug Fixes

* Fixed documentation index redirection to plugin about page
* Resolved project packaging issues
* Fixed README.md links and formatting
* Corrected markdown links throughout documentation
* Fixed CONSTRIBUTING.md formatting and section links

### 📚 Documentation

* **New Documentation Files**
  * Added DEVELOPMENT.md with development guidelines
  * Created CODE_OF_CONDUCT.md with community guidelines
  * Added CONTRIBUTING.md with contribution guidelines
  * Implemented bug_report.md, feature_request.md, and question.md templates

* **Documentation Structure**
  * Organized documentation into logical sections
  * Added detailed technical diagrams
  * Improved navigation and readability
  * Added code examples and usage patterns

### 🔧 Technical Details

* **Architecture**
  * Refined modular architecture with four main components:
    * Core Runtime Module
    * Runtime Utilities Module
    * Editor Module
    * Editor Utilities Module
  * Improved class hierarchy and relationships
  * Enhanced object variable system

* **Build System**
  * Updated CI/CD pipeline
  * Improved documentation build process
  * Added platform-specific configurations

### 📋 Known Issues

* Some platforms may require additional configuration
* Documentation site may have temporary redirect issues during deployment

### 🔜 Future Plans

* **Object Variable System Enhancements**
  * Custom Details Customization for object variables
  * Improved default value appearance with dropdown menus for type and count
  * Implementation of variable identifier using FGuid
  * Universal Get and Set functions for C++ using template arguments
  * Ability to save object variables to disk via serialization
  * Support for nesting object variables with parent-child relationships
  * Visualization of nested variables in a dedicated window (gorgeous.ov.list)
  * Implementation of persistent and non-persistent variables
  * Automatic cleanup of non-persistent variables after level switches

* **Functional Structures**
  * Enhanced structures with PostEditPropertyChange functionality
  * Improved property change callbacks for structure members
  * Better integration with the editor's property system

* **Conditional Object Choosers**
  * Implementation of condition-based object selection
  * Support for various condition types (Gameplay Tags, validity checks)
  * Branching logic based on condition results
  * Integration with the object variable system

* **Build System Improvements**
  * Internal Build.cs simplification tool
  * Internal Target.cs simplification tool
  * Automatic module dependency scanning and inclusion
  * Support for third-party library integration
  * Enhanced development workflow for both Gorgeous plugins and standalone projects

* **Documentation and Examples**
  * Expanded documentation with more detailed examples
  * Tutorial content for common use cases
  * Performance optimization guides
  * Best practices for plugin integration

---

*For detailed information about specific changes, please refer to the [GitHub commit history](https://github.com/Epic-Nova/Gorgeous-Core/commits/v0.8).*