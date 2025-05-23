# ✨ Boolean Condition (Blueprint & C++)

???+ info "Short Description"

    The `UGorgeousBooleanCondition` class evaluates boolean values based on a specified mode, providing conditional logic for the Gorgeous Things Conditional Object Chooser system.

??? info "Long Description"

    `UGorgeousBooleanCondition` extends the base Condition class to provide boolean-specific logic evaluation. This class enables simple Boolean operations between two inputs (A and B) based on the selected conditional mode, allowing for fundamental logic branching within the Gorgeous Things Conditional Object Chooser framework.

##   🚀 Features

### `CheckCondition`
=== "📝 Function Details"

    <div class="function-description">

    Evaluates the boolean condition based on the selected mode. This function processes the Boolean inputs A and B according to the specified mode and returns a value indicating the result.

    </div>

    === "Output"

        |   Parameter     |   Type    |   Description                                                                  |
        | :-------------- | :-------- | :----------------------------------------------------------------------------- |
        |   `ReturnType`  |   `uint8` |   Returns 1 if the condition evaluates to true, 0 otherwise based on the mode. |
    
    ??? note "Important"

        Ensure that the Boolean inputs A and B are properly initialized before calling this function. The evaluation result depends on the mode selected for the condition.

=== "📚 Usage Examples"

    === "C++"

        ```cpp hl_lines="6"
        UGorgeousBooleanCondition* BooleanCondition = NewObject<UGorgeousBooleanCondition>();
        
        // Set up the condition
        BooleanCondition->A = true;
        BooleanCondition->B = false;
        uint8 Result = BooleanCondition->CheckCondition();
        ```

---

###   Variable Properties

|   Property   |   Type    |   Description                                           |
| :----------- | :-------- | :------------------------------------------------------ |
|   `A`        |   `bool`  |   The first boolean input for the condition evaluation. |
|   `B`        |   `bool`  |   The second boolean input for the condition evaluation.|

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