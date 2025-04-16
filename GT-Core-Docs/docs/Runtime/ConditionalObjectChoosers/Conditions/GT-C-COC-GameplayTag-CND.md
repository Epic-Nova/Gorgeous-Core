# ✨ GameplayTag Condition (Blueprint & C++)

???+ info "Short Description"

    The `UGorgeousGameplayTagCondition` class evaluates gameplay tag values based on their appearance in a container, providing a powerful tag-based conditional system within the Gorgeous Things ecosystem.

??? info "Long Description"

    `UGorgeousGameplayTagCondition` extends the base Condition class to provide gameplay tag-specific logic evaluation. This class allows for dynamic decision-making based on the presence of specific gameplay tags within a referenced container. It supports multiple resolution strategies when multiple matching tags are found, including first match, last match, random selection, or custom rule-based evaluation.

##   🚀 Features

### `CheckCondition`
=== "📝 Function Details"

    <div class="function-description">

    Evaluates the gameplay tag condition based on the selected mode. This function checks for the presence of specified gameplay tags in the container and returns an index value based on the mapping and resolution strategy.

    </div>

    === "Output"

        |   Parameter     |   Type    |   Description                                                                           |
        | :-------------- | :-------- | :-------------------------------------------------------------------------------------- |
        |   `ReturnType`  |   `uint8` |   Returns the index corresponding to the matched condition based on the gameplay tags.  |
    
    ??? note "Important"

        Ensure that the `GameplayTagContainerClassReference` and `GameplayTagContainerUPropertyName` are properly set to reference a valid gameplay tag container. The return value depends on the `GameplayTagConditionMapping` and the resolution strategy specified in `GameplayTagChooserFightMode`.

=== "📚 Usage Examples"

    === "C++"

        ```cpp hl_lines="7"
        UGorgeousGameplayTagCondition* TagCondition = NewObject<UGorgeousGameplayTagCondition>();
        
        // Set up the condition
        TagCondition->GameplayTagContainerClassReference = SomeObject;
        TagCondition->GameplayTagContainerUPropertyName = TEXT("MyTagContainer");
        TagCondition->GameplayTagChooserFightMode = EConditionalGameplayTagChooserFightMode_E::FIRST;
        uint8 Result = TagCondition->CheckCondition();
        ```
    
    === "Blueprint"

        <figure markdown="span">
        ![Image title](./../../images/GorgeousCore/ConditionalObjectChoosers/Conditions/GameplayTagCheckCondition.png){ width="100%" }
        <figcaption>Evaluates the gameplay tag condition based on the tags in the referenced container.</figcaption>
        </figure>

### `EvaluateCustomRule`
=== "📝 Function Details"

    <div class="function-description">

    Provides a customizable blueprint-implementable method for evaluating which condition should be selected. This function can be overridden in blueprints to implement custom logic for resolving which tag-based condition to select.

    </div>

    === "Output"

        |   Parameter     |   Type    |   Description                                                                  |
        | :-------------- | :-------- | :----------------------------------------------------------------------------- |
        |   `ReturnType`  |   `uint8` |   Returns a valid index for a condition based on custom evaluation logic.      |
    
    ??? note "Important"

        This function is only called when `GameplayTagChooserFightMode` is set to `RULE`. If this function is not implemented in a blueprint child class, it will return 0 by default.

=== "📚 Usage Examples"

    === "Blueprint"

        <figure markdown="span">
        ![Image title](./../../images/GorgeousCore/ConditionalObjectChoosers/Conditions/EvaluateCustomRule.png){ width="100%" }
        <figcaption>A blueprint implementation of the custom rule evaluation function.</figcaption>
        </figure>

---

###   Variable Properties

|   Property                        |   Type                                               |   Description                                                                                |
| :-------------------------------- | :--------------------------------------------------- | :------------------------------------------------------------------------------------------- |
|   `GameplayTagContainerClassReference` | `UObject*`                                      |   Reference to the class that contains the gameplay tag container property.                   |
|   `GameplayTagContainerUPropertyName`  | `FName`                                         |   The name of the property that holds the gameplay tag container within the referenced class. |
|   `GameplayTagConditionMapping`        | `TMap<FGameplayTagContainerWrapper_S, int32>`   |   Maps gameplay tag containers to condition indices. When a key container's tags are present in the referenced container, its value is returned. |
|   `GameplayTagChooserFightMode`        | `EConditionalGameplayTagChooserFightMode_E`    |   Determines how to resolve conflicts when multiple gameplay tags match in the container.      |

### Fight Mode Options

|   Mode      |   Description                                                                                   |
| :---------- | :---------------------------------------------------------------------------------------------- |
|   `FIRST`   |   Returns the first matching tag found in the gameplay tag container.                           |
|   `LAST`    |   Returns the last matching tag found in the gameplay tag container.                            |
|   `RANDOM`  |   Returns a randomly selected matching tag from the gameplay tag container.                     |
|   `RULE`    |   Uses the custom rule defined in the `EvaluateCustomRule` function to determine which tag to select. |

<style>
.function-description {
    margin-top: 0.5em;
    font-style: italic;
    color: #555;
}
</style>