---
name: Feature Request
about: Suggest a new feature or enhancement for Gorgeous Core.
description: Suggest a new feature or enhancement for Gorgeous Core.
title: "[Feature Request]: "
labels: ["#01.1 Triage", "#11.1 Feature Request"]
assignees:
  - Epgenix
body:
  - type: markdown
    attributes:
      value: |
        🙌 **Thanks for helping improve Gorgeous Core!**  
        Please fill out this form to suggest a new feature or enhancement for the plugin. The more detail, the better! 💡

  - type: input
    id: contact
    attributes:
      label: Contact Details
      description: How can we reach you if we need more information?
      placeholder: ex. email@example.com or @yourhandle
    validations:
      required: false

  - type: textarea
    id: summary
    attributes:
      label: Feature Summary
      description: A short, clear description of the feature you'd like to see.
      placeholder: ex. Add dynamic module registration for plugin-based systems
    validations:
      required: true

  - type: textarea
    id: motivation
    attributes:
      label: Why is this feature needed?
      description: Tell us why this would be helpful, and what problem it solves.
      placeholder: ex. This would allow developers to create more modular plugin architectures...
    validations:
      required: true

  - type: textarea
    id: solution
    attributes:
      label: Suggested Solution (Optional)
      description: If you have ideas on how it could be implemented in C++, C#, or Unreal Engine – share them here!
      placeholder: ex. Add a macro like UFUNCTION(Reflectable) that registers methods to a global metadata registry.
    validations:
      required: false

  - type: textarea
    id: alternatives
    attributes:
      label: Alternatives or Workarounds
      description: Have you tried other solutions, or are there any partial workarounds that exist?
      placeholder: ex. I'm using custom delegates, but they require a lot of boilerplate and manual linking.
    validations:
      required: false

  - type: textarea
    id: use-cases
    attributes:
      label: Example Use Cases
      description: How would this feature be used in real projects or scenarios?
      placeholder: ex. Plugin-based menu systems, dynamically generating UI from reflected functions, etc.
    validations:
      required: false

  - type: textarea
    id: additional-context
    attributes:
      label: Additional Context or Resources
      description: Add screenshots, diagrams, links to libraries, or references that inspired the feature.
    validations:
      required: false

  - type: checkboxes
    id: terms
    attributes:
      label: Code of Conduct
      description: By submitting this issue, you agree to follow our [Code of Conduct](./CODE_OF_CONDUCT.md).
      options:
        - label: I agree to follow this project's Code of Conduct
          required: true
---

<!-- FEATURE REQUEST TEMPLATE -->

# 🌟 Feature Request – Gorgeous Core

Thanks for taking the time to suggest a feature for **Gorgeous Core**!  
We're excited to hear your ideas on how we can make the plugin even more powerful for Unreal Engine developers. 💬🎮

---

## 📌 Feature Summary

**What's the feature you'd like to see?**  
Describe it in a short and clear sentence.

_Example: Add support for runtime reflection metadata in Blueprint-exposed C++ functions._

---

## 🧠 Motivation

**Why is this feature important? What problem does it solve?**  
This helps us understand the context and urgency.

_Example: This would help developers dynamically inspect available functions for plugins that load modules at runtime._

---

## 🛠️ Proposed Solution

**How do you think this could be implemented (in C++, C#, or both)?**  
Feel free to drop Unreal-specific implementation hints or patterns.

_Example: A macro like `UFUNCTION(Reflectable)` could register the function with the module’s metadata registry during `StartupModule()`._

---

## 💡 Alternative Approaches

**Have you tried workarounds? Do any partial solutions exist?**

_Example: Manually registering function names in a global registry, but it's error-prone and hard to maintain._

---

## 🧪 Use Cases

**How would this feature be used in real projects or sample scenarios?**

- Dynamically binding UI buttons to Blueprint-exposed methods.
- Enabling visual scripting support in external tools.

---

## 📎 Additional Info / References

Add any extra resources, screenshots, code snippets, or links to similar tools/libraries that inspired the idea.

---

## 🙌 _Thanks for helping improve Gorgeous Core!_

> [!NOTE]
> Check out our [CONTRIBUTING.md](https://github.com/Epic-Nova/Gorgeous-Core/blob/master/CONTRIBUTING.md) for info on how to get involved.

> [!WARNING]
> By submitting this issue, you agree to follow our [Code of Conduct](https://github.com/Epic-Nova/Gorgeous-Core/blob/master/CODE_OF_CONDUCT.md).