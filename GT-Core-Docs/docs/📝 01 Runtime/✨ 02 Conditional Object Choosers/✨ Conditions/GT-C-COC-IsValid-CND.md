# ✨ IsValid Condition (Blueprint & C++)

???+ info "Short Description"

    The `UGorgeousIsValidCondition` class evaluates object validity (non-null status) to provide conditional selection logic within the Gorgeous Things ecosystem.

??? info "Long Description"

    `UGorgeousIsValidCondition` extends the base Condition class to provide object validity checking. This class enables decision-making based on whether specified object references are valid (not null), allowing for safer object handling and conditional branching based on object existence within the Gorgeous Things Conditional Object Chooser framework.

##   🚀 Features

### `CheckCondition`
=== "📝 Function Details"

    <div class="function-description">

    Checks if the object inputs are valid (not null) based on the selected mode. This function evaluates the validity of objects A and B according to the specified conditional chooser mode and returns a value indicating the result.

    </div>

    === "Output"

        |   Parameter     |   Type    |   Description                                                                  |
        | :-------------- | :-------- | :----------------------------------------------------------------------------- |
        |   `ReturnType`  |   `uint8` |   Returns 1 if the condition evaluates to true, 0 otherwise based on the mode. |
    
    ??? note "Important"

        The evaluation is based on the `Mode` property inherited from `UGorgeousCondition`, which determines how the validity of objects A and B are combined. For example, if Mode is set to AND, both objects must be valid for the condition to return 1.

=== "📚 Usage Examples"

    === "C++"

        ```cpp hl_lines="6"
        UGorgeousIsValidCondition* ValidityCondition = NewObject<UGorgeousIsValidCondition>();
        
        // Set up the condition
        ValidityCondition->A = SomeObject;
        ValidityCondition->B = AnotherObject;
        uint8 Result = ValidityCondition->CheckCondition();
        ```
    
    === "Blueprint"

        <figure markdown="span">
        ![Image title](./../../images/GorgeousCore/ConditionalObjectChoosers/Conditions/IsValidCheckCondition.png){ width="100%" }
        <figcaption>Evaluates if the provided objects are valid based on the selected mode.</figcaption>
        </figure>

---

###   Variable Properties

|   Property   |   Type       |   Description                                      |
| :----------- | :----------- | :------------------------------------------------- |
|   `A`        |   `UObject*` |   The first object input to check for validity.    |
|   `B`        |   `UObject*` |   The second object input to check for validity.   |

### Condition Modes (Inherited from UGorgeousCondition)

|   Mode        |   Description                                                                      |
| :------------ | :--------------------------------------------------------------------------------- |
|   `AND`       |   Both objects A and B must be valid.                                              |
|   `OR`        |   Either object A or B (or both) must be valid.                                    |
|   `XOR`       |   Either object A or B must be valid, but not both.                                |
|   `NAND`      |   Both objects A and B must not be valid together.                                 |
|   `NOR`       |   Neither object A nor B can be valid.                                             |
|   `A_ONLY`    |   Only checks if object A is valid.                                                |
|   `B_ONLY`    |   Only checks if object B is valid.                                                |
|   `N_A_ONLY`  |   Only checks if object A is not valid.                                            |
|   `N_B_ONLY`  |   Only checks if object B is not valid.                                            |

<style>
.function-description {
    margin-top: 0.5em;
    font-style: italic;
    color: #555;
}
</style>