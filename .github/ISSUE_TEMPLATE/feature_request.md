---
name: Feature request
about: Suggest an idea for this project
title: ''
labels: ''
assignees: ''

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

> ⚠️ _Note:_ Gorgeous Core is primarily written in **C++**, but we are considering **C# integrations** through third-party bridges. If your request involves C#, please specify the interoperability scope.

---

> 🙌 _Thanks for helping improve Gorgeous Core!_  
> Check out our [CONTRIBUTING.md](./CONTRIBUTING.md) for info on how to get involved.
