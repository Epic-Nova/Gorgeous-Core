# 🔎 About Conditional Object Choosers

???+ info "Short Description"

    The Gorgeous Conditional Object Chooser system provides a mechanism to dynamically select Object Variables based on various condition evaluations, enabling complex decision-making and conditional branching within the Gorgeous Things ecosystem.

??? info "Long Description"

    The Gorgeous Conditional Object Chooser system enables developers to create sophisticated decision-making components that dynamically select Object Variables based on various conditions. This system allows for flexible and responsive data flow control in Unreal Engine projects.
    
    Key features include:

    * **Conditional Selection:** Selects Object Variables based on evaluated conditions.
    * **Multiple Condition Types:** Supports various condition types including Boolean, IsValid, and GameplayTag conditions.
    * **Custom Condition Support:** Allows for the creation of custom condition types to extend functionality.
    * **Blueprint Compatibility:** Fully accessible and configurable in both C++ and Blueprint.
    * **Integration with Object Variables:** Seamlessly works with the Gorgeous Object Variable system.
    * **Logic Operations:** Supports standard logical operations (AND, OR, XOR, NAND, NOR) and specialized condition checking.
    * **Tag-Based Selection:** Enables selection based on GameplayTag presence and evaluation.
    * **Custom Rule Evaluation:** Provides customizable rules for complex selection logic.

## 🤔 Frequently Asked Questions (FAQ)

**Question:** What types of conditions does the Conditional Object Chooser support?

**Answer:** It supports multiple condition types including Boolean conditions (evaluating true/false values), IsValid conditions (checking if object references are valid), and GameplayTag conditions (evaluating based on GameplayTag presence), as well as custom conditions that can be created by extending the base condition class.

**Question:** How does the Conditional Object Chooser relate to the Object Variable system?

**Answer:** The Conditional Object Chooser system works in conjunction with the Object Variable system, allowing for dynamic selection of Object Variables based on specified conditions. It provides a way to branch between different Object Variables based on runtime state.

**Question:** Can I create custom condition types for specific project needs?

**Answer:** Yes, you can create custom conditions by extending the UGorgeousCondition base class and implementing the CheckCondition function to define your own evaluation logic.

**Question:** How are multiple matching conditions handled in the GameplayTag condition?

**Answer:** The GameplayTag condition provides several resolution strategies through the GameplayTagChooserFightMode property, including selecting the first match, last match, a random match, or using a custom rule-based approach defined in the EvaluateCustomRule function.
