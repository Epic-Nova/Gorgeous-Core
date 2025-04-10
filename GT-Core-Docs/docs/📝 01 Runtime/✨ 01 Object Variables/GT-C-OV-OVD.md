# ✨ Object Variable Definitions (Blueprint & C++)

???+ info "Short Description"

    The `GorgeousObjectVariableDefinitions.h` file defines a comprehensive set of object variable classes that extend the base `UGorgeousObjectVariable` class. These classes provide specialized implementations for different data types and collection types (single, array, map, set).

??? info "Long Description"

    The object variable definitions provide a structured way to represent various data types as UObjects within the Gorgeous Things ecosystem. These classes are organized into categories based on their data type and collection type, making it easy to find and use the appropriate variable class for your needs.

    Each object variable class implements specific interfaces that provide getter and setter functions for the data they store. This allows for consistent access patterns across different variable types and enables easy integration with Blueprints.

## 🚀 Variable Categories

### Base Classes

| Class | Description |
|-------|-------------|
| `USingleObjectVariable` | Abstract base class for single object variables. |
| `UArrayObjectVariable` | Abstract base class for array object variables. |
| `UMapObjectVariable` | Abstract base class for map object variables. |
| `USetObjectVariable` | Abstract base class for set object variables. |

### Object Type Base Classes

| Class | Description |
|-------|-------------|
| `USingleObjectTypeObjectVariable` | Abstract base class for single object variables that hold object type values. |
| `UArrayObjectTypeObjectVariable` | Abstract base class for array object variables that hold object type values. |
| `UMapObjectTypeObjectVariable` | Abstract base class for map object variables that hold object type keys. |
| `USetObjectTypeObjectVariable` | Abstract base class for set object variables that hold object type values. |

## 📊 Single Object Variables

Single object variables hold a single value of a specific type.

### Object Type Single Variables

| Class | Description | Value Type |
|-------|-------------|------------|
| `UObject_SOTOV` | Object Single Object Variable | `UObject*` |
| `UClass_SOTOV` | Class Single Object Variable | `UClass*` |
| `USoftObject_SOTOV` | Soft Object Single Object Variable | `TSoftObjectPtr<UObject>` |
| `USoftClass_SOTOV` | Soft Class Single Object Variable | `TSoftClassPtr<UObject>` |

### Primitive Type Single Variables

| Class | Description | Value Type |
|-------|-------------|------------|
| `UBoolean_SOV` | Boolean Single Object Variable | `bool` |
| `UByte_SOV` | Byte Single Object Variable | `uint8` |
| `UFloat_SOV` | Float Single Object Variable | `double` |
| `UInteger64_SOV` | Integer64 Single Object Variable | `int64` |
| `UInteger_SOV` | Integer Single Object Variable | `int32` |
| `UName_SOV` | Name Single Object Variable | `FName` |
| `URotator_SOV` | Rotator Single Object Variable | `FRotator` |
| `UString_SOV` | String Single Object Variable | `FString` |
| `UText_SOV` | Text Single Object Variable | `FText` |
| `UTransform_SOV` | Transform Single Object Variable | `FTransform` |
| `UVector_SOV` | Vector Single Object Variable | `FVector` |

## 📊 Array Object Variables

Array object variables hold an array of values of a specific type.

### Object Type Array Variables

| Class | Description | Value Type |
|-------|-------------|------------|
| `UObject_AOTOV` | Object Array Object Variable | `TArray<UObject*>` |
| `UClass_AOTOV` | Class Array Object Variable | `TArray<UClass*>` |
| `USoftObject_AOTOV` | Soft Object Array Object Variable | `TArray<TSoftObjectPtr<UObject>>` |
| `USoftClass_AOTOV` | Soft Class Array Object Variable | `TArray<TSoftClassPtr<UObject>>` |

### Primitive Type Array Variables

| Class | Description | Value Type |
|-------|-------------|------------|
| `UBoolean_AOV` | Boolean Array Object Variable | `TArray<bool>` |
| `UByte_AOV` | Byte Array Object Variable | `TArray<uint8>` |
| `UFloat_AOV` | Float Array Object Variable | `TArray<double>` |
| `UInteger64_AOV` | Integer64 Array Object Variable | `TArray<int64>` |
| `UInteger_AOV` | Integer Array Object Variable | `TArray<int32>` |
| `UName_AOV` | Name Array Object Variable | `TArray<FName>` |
| `URotator_AOV` | Rotator Array Object Variable | `TArray<FRotator>` |
| `UString_AOV` | String Array Object Variable | `TArray<FString>` |
| `UText_AOV` | Text Array Object Variable | `TArray<FText>` |
| `UTransform_AOV` | Transform Array Object Variable | `TArray<FTransform>` |
| `UVector_AOV` | Vector Array Object Variable | `TArray<FVector>` |

## 📊 Map Object Variables

Map object variables hold a map where the key is of a specific type and the value is a `UGorgeousObjectVariable`.

### Object Type Map Variables

| Class | Description | Key Type |
|-------|-------------|----------|
| `UObject_MOTOV` | Object Map Object Variable | `UObject*` |
| `UClass_MOTOV` | Class Map Object Variable | `UClass*` |
| `USoftObject_MOTOV` | Soft Object Map Object Variable | `TSoftObjectPtr<UObject>` |
| `USoftClass_MOTOV` | Soft Class Map Object Variable | `TSoftClassPtr<UObject>` |

### Primitive Type Map Variables

| Class | Description | Key Type |
|-------|-------------|----------|
| `UByte_MOV` | Byte Map Object Variable | `uint8` |
| `UFloat_MOV` | Float Map Object Variable | `double` |
| `UInteger64_MOV` | Integer64 Map Object Variable | `int64` |
| `UInteger_MOV` | Integer Map Object Variable | `int32` |
| `UName_MOV` | Name Map Object Variable | `FName` |
| `UString_MOV` | String Map Object Variable | `FString` |
| `UTransform_MOV` | Transform Map Object Variable | `FTransform` |
| `UVector_MOV` | Vector Map Object Variable | `FVector` |

## 📊 Set Object Variables

Set object variables hold a set of values of a specific type.

### Object Type Set Variables

| Class | Description | Value Type |
|-------|-------------|------------|
| `UObject_STOTOV` | Object Set Object Variable | `TSet<UObject*>` |
| `UClass_STOTOV` | Class Set Object Variable | `TSet<UClass*>` |
| `USoftObject_STOTOV` | Soft Object Set Object Variable | `TSet<TSoftObjectPtr<UObject>>` |
| `USoftClass_STOTOV` | Soft Class Set Object Variable | `TSet<TSoftClassPtr<UObject>>` |

### Primitive Type Set Variables

| Class | Description | Value Type |
|-------|-------------|------------|
| `UByte_STOV` | Byte Set Object Variable | `TSet<uint8>` |
| `UFloat_STOV` | Float Set Object Variable | `TSet<double>` |
| `UInteger64_STOV` | Integer64 Set Object Variable | `TSet<int64>` |
| `UInteger_STOV` | Integer Set Object Variable | `TSet<int32>` |
| `UName_STOV` | Name Set Object Variable | `TSet<FName>` |
| `UString_STOV` | String Set Object Variable | `TSet<FString>` |
| `UTransform_STOV` | Transform Set Object Variable | `TSet<FTransform>` |
| `UVector_STOV` | Vector Set Object Variable | `TSet<FVector>` |

## 🚀 Interface Functions

Each object variable class implements specific interfaces that provide getter and setter functions for the data they store. These functions are automatically generated using the `UE_DEFINE_OBJECT_VARIABLE_*_INTERFACE` macros.

### Single Object Variable Interface Functions

#### Getter Functions

| Function | Description | Return Type |
|----------|-------------|-------------|
| `GetObjectVariableSingleObjectVariable` | Gets a GorgeousObjectVariable | `UGorgeousObjectVariable*` |
| `GetObjectObjectSingleObjectVariable` | Gets a UObject | `UObject*` |
| `GetObjectClassSingleObjectVariable` | Gets a UClass | `UClass*` |
| `GetSoftObjectObjectSingleObjectVariable` | Gets a TSoftObjectPtr<UObject> | `TSoftObjectPtr<UObject>` |
| `GetSoftObjectClassSingleObjectVariable` | Gets a TSoftClassPtr<UObject> | `TSoftClassPtr<UObject>` |
| `GetBooleanSingleObjectVariable` | Gets a boolean | `bool` |
| `GetByteSingleObjectVariable` | Gets a byte | `uint8` |
| `GetFloatSingleObjectVariable` | Gets a double | `double` |
| `GetInteger64SingleObjectVariable` | Gets a 64-bit integer | `int64` |
| `GetIntegerSingleObjectVariable` | Gets an integer | `int32` |
| `GetNameSingleObjectVariable` | Gets an FName | `FName` |
| `GetRotatorSingleObjectVariable` | Gets an FRotator | `FRotator` |
| `GetStringSingleObjectVariable` | Gets an FString | `FString` |
| `GetTextSingleObjectVariable` | Gets an FText | `FText` |
| `GetTransformSingleObjectVariable` | Gets an FTransform | `FTransform` |
| `GetVectorSingleObjectVariable` | Gets an FVector | `FVector` |

#### Setter Functions

| Function | Description | Return Type |
|----------|-------------|-------------|
| `SetObjectVariableSingleObjectVariable` | Sets a GorgeousObjectVariable | `UGorgeousObjectVariable*` |
| `SetObjectObjectSingleObjectVariable` | Sets a UObject | `UObject*` |
| `SetObjectClassSingleObjectVariable` | Sets a UClass | `UClass*` |
| `SetSoftObjectObjectSingleObjectVariable` | Sets a TSoftObjectPtr<UObject> | `TSoftObjectPtr<UObject>` |
| `SetSoftObjectClassSingleObjectVariable` | Sets a TSoftClassPtr<UObject> | `TSoftClassPtr<UObject>` |
| `SetBooleanSingleObjectVariable` | Sets a boolean | `bool` |
| `SetByteSingleObjectVariable` | Sets a byte | `uint8` |
| `SetFloatSingleObjectVariable` | Sets a double | `double` |
| `SetInteger64SingleObjectVariable` | Sets a 64-bit integer | `int64` |
| `SetIntegerSingleObjectVariable` | Sets an integer | `int32` |
| `SetNameSingleObjectVariable` | Sets an FName | `FName` |
| `SetRotatorSingleObjectVariable` | Sets an FRotator | `FRotator` |
| `SetStringSingleObjectVariable` | Sets an FString | `FString` |
| `SetTextSingleObjectVariable` | Sets an FText | `FText` |
| `SetTransformSingleObjectVariable` | Sets an FTransform | `FTransform` |
| `SetVectorSingleObjectVariable` | Sets an FVector | `FVector` |

### Array Object Variable Interface Functions

Array object variables implement similar getter and setter functions for arrays, with additional functions for array-specific operations such as adding, removing, and finding elements.

### Map Object Variable Interface Functions

Map object variables implement similar getter and setter functions for maps, with additional functions for map-specific operations such as adding, removing, and finding key-value pairs.

### Set Object Variable Interface Functions

Set object variables implement similar getter and setter functions for sets, with additional functions for set-specific operations such as adding, removing, and finding elements.

## 📚 Usage Examples

### Creating and Using a Single Object Variable

=== "C++"

    ```cpp hl_lines="8 11"
    // Create a new string object variable
    UGorgeousRootObjectVariable* RootObjectVariable = UGorgeousRootObjectVariable::GetRootObjectVariable();
    FGuid StringVariableIdentifier;
    UGorgeousObjectVariable* StringVariable = RootObjectVariable->NewObjectVariable(UString_SOV::StaticClass(), StringVariableIdentifier, nullptr, false);
    
    // Set the value
    FString Value = FString("Hello, World!");
    IGorgeousSingleObjectVariablesSetter_I::Execute_SetStringSingleObjectVariable(StringVariable, NAME_None, Value);
    
    // Get the value
    FString RetrievedValue = IGorgeousSingleObjectVariablesGetter_I::Execute_GetStringSingleObjectVariable(StringVariable, NAME_None);
    ```

=== "Blueprint"

    <figure markdown="span">
    ![Image title](./../../images/GorgeousCore/ObjectVariables/StringVariableExample.png){ width="100%" }
    <figcaption>Creating and using a String Single Object Variable in Blueprint.</figcaption>
    </figure>

### Creating and Using an Array Object Variable

=== "C++"

    ```cpp hl_lines="8 11"
    // Create a new integer array object variable
    UGorgeousRootObjectVariable* RootObjectVariable = UGorgeousRootObjectVariable::GetRootObjectVariable();
    FGuid IntegerArrayVariableIdentifier;
    UGorgeousObjectVariable* IntegerArrayVariable = RootObjectVariable->NewObjectVariable(UInteger_AOV::StaticClass(), IntegerArrayVariableIdentifier, nullptr, false);
    
    // Set the value
    TArray<int32> Values = {1, 2, 3, 4, 5};
    IGorgeousArrayObjectVariablesSetter_I::Execute_SetIntegerArrayObjectVariable(IntegerArrayVariable, NAME_None, Values);
    
    // Get the value
    TArray<int32> RetrievedValues = IGorgeousArrayObjectVariablesGetter_I::Execute_GetIntegerArrayObjectVariable(IntegerArrayVariable, NAME_None);
    ```

=== "Blueprint"

    <figure markdown="span">
    ![Image title](./../../images/GorgeousCore/ObjectVariables/IntegerArrayVariableExample.png){ width="100%" }
    <figcaption>Creating and using an Integer Array Object Variable in Blueprint.</figcaption>
    </figure>

### Creating and Using a Map Object Variable

=== "C++"

    ```cpp hl_lines="15 18"
	// Create a new string map object variable
	UGorgeousRootObjectVariable* RootObjectVariable = UGorgeousRootObjectVariable::GetRootObjectVariable();
	FGuid StringMapVariableIdentifier;
	UGorgeousObjectVariable* StringMapVariable = RootObjectVariable->NewObjectVariable(UString_MOV::StaticClass(), StringMapVariableIdentifier, nullptr, false);
    
	// Set the value
	FGuid MapValueVariableIdentifier;
	UGorgeousObjectVariable* MapValueVariable = RootObjectVariable->NewObjectVariable(UString_SOV::StaticClass(), MapValueVariableIdentifier, StringMapVariable, false);
	FString MapValue = FString("Hello, World!");
	IGorgeousSingleObjectVariablesSetter_I::Execute_SetStringSingleObjectVariable(MapValueVariable, NAME_None, MapValue);

	TMap<FString, UGorgeousObjectVariable*> Value = TMap<FString, UGorgeousObjectVariable*>({
		{ FString("Key1"), MapValueVariable }
	});
	IGorgeousMapObjectVariablesSetter_I::Execute_SetStringMapObjectVariable(StringMapVariable, NAME_None, Value);
    
	// Get the value
	TMap<FString, UGorgeousObjectVariable*> RetrievedMap = IGorgeousMapObjectVariablesGetter_I::Execute_GetStringMapObjectVariable(StringMapVariable, NAME_None);
    ```

=== "Blueprint"

    <figure markdown="span">
    ![Image title](./../../images/GorgeousCore/ObjectVariables/StringMapVariableExample.png){ width="100%" }
    <figcaption>Creating and using a String Map Object Variable in Blueprint.</figcaption>
    </figure>

### Creating and Using a Set Object Variable

=== "C++"

    ```cpp hl_lines="10 13"
    // Create a new name set object variable
    UGorgeousRootObjectVariable* RootObjectVariable = UGorgeousRootObjectVariable::GetRootObjectVariable();
    FGuid NameSetVariableIdentifier;
    UGorgeousObjectVariable* NameSetVariable = RootObjectVariable->NewObjectVariable(UName_STOV::StaticClass(), NameSetVariableIdentifier, nullptr, false);
    
    // Set the value
    TSet<FName> Value = TSet<FName>({
        "Name1"
    });
    IGorgeousSetObjectVariablesSetter_I::Execute_SetNameSetObjectVariable(NameSetVariable, NAME_None, Value);
    
     // Get the value
    TSet<FName> RetrievedSet = IGorgeousSetObjectVariablesGetter_I::Execute_GetNameSetObjectVariable(NameSetVariable, NAME_None);
    ```

=== "Blueprint"

    <figure markdown="span">
    ![Image title](./../../images/GorgeousCore/ObjectVariables/NameSetVariableExample.png){ width="100%" }
    <figcaption>Creating and using a Name Set Object Variable in Blueprint.</figcaption>
    </figure>

!!! tip

    Specifying the OptionalVariableName variable in any interface Set/Get function, allows you to target not just the default "Value" UProperty, but also any UProperty inside the desired UObject that this function is being called on.

## 🔧 Common Properties

All object variable classes inherit the following properties from `UGorgeousObjectVariable`:

| Property | Type | Description |
|----------|------|-------------|
| `UniqueIdentifier` | `FGuid` | A unique identifier for the object variable. |
| `VariableRegistry` | `TArray<TObjectPtr<UGorgeousObjectVariable>>` | The registry of child object variables. |
| `bPersistent` | `bool` | Indicates whether the variable persists across level transitions. |
| `Parent` | `UGorgeousObjectVariable*` | The parent object variable in the hierarchy. |

Each object variable class also has a `Value` property of the appropriate type for storing the actual data.

## 🔍 Implementation Details

The object variable classes use the following macros to automatically generate getter and setter functions:

- `UE_DEFINE_OBJECT_VARIABLE_SINGLE_INTERFACE_IMPLEMENTATION` - For single object variables
- `UE_DEFINE_OBJECT_VARIABLE_MULTIPLE_INTERFACE_IMPLEMENTATION` - For array and set object variables
- `UE_DEFINE_OBJECT_VARIABLE_MAP_INTERFACE_IMPLEMENTATION` - For map object variables

These macros simplify the process of defining how object variables interact with different data types and ensure consistent behavior across all variable types.

<style>
.function-description {
    margin-top: 0.5em;
    font-style: italic;
    color: #555;
}
</style>