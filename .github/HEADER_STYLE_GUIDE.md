# Gorgeous Header & Source Style Guide

This file is the **authoritative, complete** specification for how `.h` and `.cpp` files in
the Gorgeous codebase must be commented and structured. Agents MUST follow it verbatim. When
in doubt, match the reference files listed at the bottom rather than improvising.

Reference files (Runtime module only — never copy Editor-only code):

- `Template.h` — universal starting template (only relevant parts are used).
- `.../GorgeousCoreRuntime/Public/ConditionalObjectChoosers/GorgeousConditionalObjectChooser.h`
  and `GorgeousConditionalObjectChooserTypes.h` — public-header style.
- `.../GorgeousCoreEditor/Private/HeaderFiles/PropertyTypeCustomizations/GorgeousObjectVariablePropertyTypeCustomization.h`
  — Private-header style.
- `.../GorgeousCoreRuntime/Public/DataSchemaMapping/GorgeousDataSchemaMappingTypes.h`
  — enums/structs/properties style.
- `.../GorgeousCoreRuntime/Private/SourceFiles/ConditionalObjectChoosers/GorgeousConditionalObjectChooser.cpp`
  — cpp style.

---

## 0. Golden rules (read first)

1. **Only include sections you actually need.** If a section/group would be empty, omit it
   entirely — do not leave empty comment blocks or placeholder text.
2. **Comments describe behaviour, they do not list features.** Write a precise, on-point
   description of what the code *does*. No filler, no bullet lists of "features".
3. **Never change code logic.** You may only add/adjust comments and the structural comment
   blocks defined here. Do not alter signatures, `UCLASS`/`UPROPERTY`/`UFUNCTION` specifiers
   (except to *merge in* the Help `meta` tags from §1), include paths, or statements.
4. **A comment always lives next to what it describes.** Member comments stay *inside* the
   class/struct body. Never place a member's comment outside the `{}`. Never leave a blank
   line between a `UCLASS`/`USTRUCT`/`UENUM` and its preceding comment block.
5. **One blank line** separates top-level blocks (banner → includes → class info → class).
   **No blank line** inside the include banner wrapper or directly under/over its divider
   lines (see §1).
6. **No trailing newline at EOF.** `.h` and `.cpp` files end on the final character of the last
   line (the closing `};` / last statement). Do not append a final `\n`. The single blank line
   that separates top-level blocks is an *interior* separator, never a trailing one.

---

## 1. File header banner

Every file (`.h` and `.cpp`) starts with this exact banner. Keep the two copyright/credit
blocks verbatim; use the current year.

```cpp
// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
/*==========================================================================>
|               Gorgeous Core - Core functionality provider                 |
| ------------------------------------------------------------------------- |
|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |
|              administrated by Epic Nova. All rights reserved.             |
| ------------------------------------------------------------------------- |
|                    Epic Nova is an independent entity,                    |
|          that is not affiliated with Epic Games in any capacity.          |
<==========================================================================*/
#pragma once
```

### Includes block

Immediately after `#pragma once`, wrap includes in this banner. **Critical formatting
rules (validated against the shipped widget headers):**

- The open divider `//<=============================--- Includes ---=============================>` is
  followed **on the very next line** by the first sub-section header. There is **NO blank line**
  directly under the open divider and **NO blank line** directly above the close divider.
- Keep only the sub-sections you use, in this order:
  1. `//<--------------------------=== Module Includes ===------------------------->` (project/plugin includes)
  2. `//<--------------------------=== Engine Includes ===------------------------->`
  3. `//--------------=== Third Party & Miscellaneous Includes ===-----------------` (the `.generated.h` include goes **last** here)
- **There are NO blank lines anywhere inside the include block.** Not between a sub-section
  header and its first `#include`, not between includes of the same sub-section, and not between
  sub-sections. The header line, its `#include`(s), the next header line, and its `#include`(s)
  all run contiguously until the closing divider.
- The block ends with `//<-------------------------------------------------------------------------->` on the
  line immediately after the last include (no blank line before it).
- The include block must be terminated by a **single blank line** that separates it from the next
  top-level block (Class Info or Forward Declarations).

**Definitive normalized example (this is the only accepted shape):**

```cpp
//<=============================--- Includes ---=============================>
//<--------------------------=== Module Includes ===------------------------->
#include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationHelperMacros.h"
//<--------------------------=== Engine Includes ===------------------------->
#include "CoreMinimal.h"
#include "Components/Overlay.h"
#include "GameplayTagContainer.h"
//--------------=== Third Party & Miscellaneous Includes ===-----------------
#include "GorgeousCommonOverlay.generated.h"
//<-------------------------------------------------------------------------->
```

If only the generated header exists, keep only the third sub-section (still with no internal blank lines).

### Help `meta` tags

Public runtime `UCLASS`es carry `meta = (...)` tags that register Help buttons. These do
**not** exist in a normal UE class — always merge them into the `UCLASS(...)` specifier for
public runtime classes (do not remove existing specifiers like `Abstract`, `Blueprintable`):

```cpp
UCLASS(Blueprintable, BlueprintType,
    meta = (
        DocumentationOverview  = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Overview",
        DocumentationAPI = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/GorgeousX",
        DocumentationExamples = "https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/GeneralSystems/CommonUIFoundation/Examples/"
        ))
```

**CRITICAL — line length:** The three doc tags must NEVER sit on one endless line. Always
break them across lines as shown above (one tag per line, indented under `meta = (`), matching
the `GorgeousConditionalObjectChooser.h` reference. A single-line `meta = (Documentation...)` is
a defect.

Use the matching module/path/class for the URLs. `UINTERFACE` classes do **not** get these
tags (they are not directly instantiated).

### Class Info block

Public runtime `UCLASS`es carry a bulky `Class Info` block placed **between the Includes block
and the `UCLASS` line**. Format rules, verbatim from `Template.h`:

```cpp
/*
<=============================--- Class Info ---============================>
<-----------------------------=== Quick Info ===---------------------------->
| Display Name: Gorgeous Interaction Foundation
| Functional Name: UGorgeousInteractionFoundation
| Parent Class: UGorgeous
| Class Suffix: -
| Author: Nils Bergemann
<--------------------------------------------------------------------------->
<--------------------------=== Class Description ===------------------------>
| Blueprint function library exposing helper functions for
| interaction related functionality in Blueprints.
<--------------------------------------------------------------------------->
<===========================================================================>
*/
UCLASS(Blueprintable, BlueprintType, meta = ( ... ))
class GORGEOUSCORERUNTIME_API UGorgeousInteractionFoundation : public UGorgeous
```

- **NO blank line** between the closing `*/` of this block and the `UCLASS`/`UINTERFACE` macro.
  They must be directly adjacent (the `*/` is on one line, `UCLASS(...)` on the next).
- Fill `Display Name`, `Functional Name`, `Parent Class`, `Class Suffix` (`-` unless the class
  has a real suffix), and `Author: Nils Bergemann` from the actual class.
- **Class Description wrapping:** the description text lines (the `| ...` rows) must wrap so no
  row exceeds the width of the `|` column in the banner. The rule of thumb is to break the line
  when it is longer than the `Class Description` banner line, which is **77 characters** wide:
  `<--------------------------=== Class Description ===------------------------>`. Each wrapped
  row starts with `| ` and continues the sentence; do not let a single `|` row run past ~77
  characters.
- **This block replaces any separate simple `/* */` class description** — do not add both.
- Struct/enum-only headers (no `UCLASS`) do **not** get a Class Info block.

---

## 2. Public runtime class (`UCLASS`)

### Forward declarations

If the file forward-declares classes (`class UFoo;`), wrap them in a `Forward Declarations`
block placed **after the Includes block (and after the Class Info block, if present), before
the class**. Format:

```cpp
//<=================--- Forward Declarations ---=================>
class UGorgeousUIState_DA;
class UGorgeousInputBinding_DA;
//<------------------------------------------------------------->
```

Only add this block when forward declarations actually exist.

### Section grouping

Group members using the `//<===...===>` divider comments, but **only the groups you use** (an
empty group is a bug — omit it). Each used group opens with its header and closes with
`//<------------------------------------------------------------------------->`. The known group headers (use verbatim):

- `//<============================--- Overrides ---============================>`
- `//<=======================--- Blueprint Functions ---=======================>`
- `//<====================--- UAT/UBT Exposed Variables ---====================>`
- `//<============================--- C++ Only ---=============================>`
- `//<============================--- Variables ---============================>`
- `//<============================--- Callbacks ---============================>`
- `//<===================--- Friend Classes ---================>`

**Member ordering and group placement (mandatory):**

1. **Constructor (and destructor) come first, before any group banner.** Place them directly
   after `GENERATED_BODY()` with a one-line `//` comment, exactly as the Template shows (lines
   59-63). Do not bury them inside a group.
2. **One access specifier per group, inside the group.** The `public:` / `protected:` /
   `private:` keyword goes on its own line immediately after the opening group banner (with a
   blank line between the banner and the specifier, matching `Template.h`). Do not scatter bare
   `public:`/`protected:` blocks outside the group banners, and do not place a member's comment
   outside the group that owns it.
3. **The `Overrides` group opens with `public:`.** Inside it, virtual overrides are organized with
   sub-group banners (`//<----------------------=== Actor Overrides ===------------------------>`,
   `//<----------------------=== UObject Overrides ===-------------------------->`,
   `//<--------------------=== Interface Overrides ===-------------------------->`), each optionally
   closed with a matching `//<----------------------=== End <Name> Overrides ===-------------------->`.
   Interface boilerplate macros (e.g. `UE_UI_WIDGET_INTERFACE_BOILERPLATE()`) go inside the
   **`Interface Overrides`** sub-group — NOT at the top of `Blueprint Functions`. Access
   specifiers may switch inside the `Overrides` group (e.g. `public:` for interface items, then
   `protected:` for the virtual overrides), each still inside the one `Overrides` group.
4. **Standard top-to-bottom group order** for a typical public runtime `UCLASS`:
   - constructor / destructor (ungrouped, first)
   - `Overrides` (opens `public:`, holds the Interface Overrides sub-group + `protected:` virtuals)
   - `Blueprint Functions` (`public:`)
   - `UAT/UBT Exposed Variables` (`public:`)
   - other groups (`Callbacks`, `C++ Only`, `Variables`, `Friend Classes`) as needed.
   When a sibling reference header in the same module already defines this ordering, match the
   sibling verbatim rather than reordering.
5. **Never drop required macros or members.** Interface boilerplate macros and all original
   `UCLASS`/`UFUNCTION`/`UPROPERTY` members must remain; only comments and the structural banners
   are added around them. Do not remove `UE_UI_WIDGET_INTERFACE_BOILERPLATE()` or similar required
   macros when re-grouping.

6. **Every group is wrapped in a `#pragma region` / `#pragma endregion`.** Immediately after the
   group's opening `//<===...===>` banner, open `#pragma region <GroupName>` (a single blank line
   between the banner and the `#pragma region` is allowed). The matching `#pragma endregion
   <GroupName>` is placed **immediately after** the group's closing `//<--------...--------->`
   divider (no blank line between the close divider and the `#pragma endregion`). The region name
   is the human-readable group label, e.g. `Overrides`, `Blueprint Functions`,
   `UAT/UBT Exposed Variables`, `C++ Only`. This lets IDEs collapse each group. Use the existing
   repo convention (`#pragma region Foo` … `#pragma endregion Foo`).

7. **Spacing rules (mandatory, easy to get wrong):**
   - **Between two top-level groups: exactly two blank lines.** The `#pragma endregion` of group A
     is followed by two blank lines, then the `//<===...===>` banner of group B.
   - **Inside a group (between the banner, the access specifier, members, sub-groups, and the
     close divider): single blank lines only.** Never two blank lines inside a group.
   - **`#pragma region` is DIRECTLY followed by the access specifier — NO blank line between
     them.** Write `#pragma region Overrides` then `public:` on the next line, with nothing in
     between. (A blank line between the group banner and `#pragma region` is allowed; a blank
     line between `#pragma region` and the access specifier is forbidden.)
   - **A group's closing divider `//<--------...--------->` ALWAYS sticks directly to whatever is
     immediately above it** — there is NEVER a blank line between the last member (or last
     sub-group close) and the group's closing divider. The same applies to sub-group closing
     banners (e.g. `//<--- End Interface Overrides --->`).
   - **Access specifiers (`public:` / `protected:` / `private:`) are flush-left (no leading tab).**
     Members inside the group keep their normal one-level indent.

`public:` / `protected:` / `private:` go **inside** the group, right after the opening banner
(one access specifier per group; do not repeat the banner per function). You may add
sub-group banners such as `//<--------------------------=== FTickableGameObject ===--------------------->`
and a matching `//<--------------------------=== End FTickableGameObject ===----------------->`.

```cpp
    //<============================--- Overrides ---============================>
    #pragma region Overrides
    public:

    // Sanitizes serialized data after load to drop placeholders created by circular blueprint dependencies.
    virtual void PostLoad() override;

    // Sanitizes during load so placeholders are nulled before dependency repair runs.
    virtual void Serialize(FArchive& Ar) override;
    //<------------------------------------------------------------------------->
    #pragma endregion Overrides


    //<=======================--- Blueprint Functions ---=======================>
    #pragma region Blueprint Functions
    public:

    // ...
    //<------------------------------------------------------------------------->
    #pragma endregion Blueprint Functions
```

### Function comments (REQUIRED `@param` / `@return`)

Every function comment uses a `/** ... */` block. **Rules that are mandatory:**

- State what the function does, in the present tense, describing behaviour (not listing).
- Add a `@param` line for **every** parameter, in declaration order.
- Add a `@return` line **only when the return type is not `void`**. For `void` and for
  constructors/destructors, omit `@return` entirely.
- For `UFUNCTION` Blueprint events with no meaningful return, still list `@param`s.

```cpp
/**
 * Selects an object variable based on the condition check.
 *
 * @param TargetActor The actor to request the interaction tags from.
 * @param OutInteractionTags Receives the requested interaction tags when the call succeeds.
 * @return True if the tags were requested successfully, false otherwise.
 */
UFUNCTION(BlueprintCallable, Category = "...")
static bool TryRequestInteractionTags(AActor* TargetActor, FGameplayTagContainer& OutInteractionTags);
```

- **Overrides / trivial one-liners:** a single-line `//` comment above is acceptable instead
  of a block.
- **C++-only internal functions:** use the same `/** ... */` block with `@param`/`@return`
  when they take parameters or return non-void; otherwise a `//` line.

### Exposed variables (`UPROPERTY`) and plain members

- **A `UPROPERTY` or plain member gets a single-line `//` comment** describing it.
- Use `/** ... */` for a property **only when the description genuinely does not fit on one
  line** (rare). Multi-line property comments for short descriptions are a bug — fix them to
  `//`.
- The comment goes **above** the `UPROPERTY`/`UFUNCTION` specifier, which stays on the line
  immediately above the member.

```cpp
    // The condition check that should be performed.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "...")
    TObjectPtr<UGorgeousCondition> ConditionCheck = nullptr;
```

### Length-based comment style (applies everywhere)

- Fits on **one line** → `// Comment.`
- Does **not** fit one line → opened block:

  ```cpp
  /**
   *
   */
  ```

  Fill the middle line(s). Never use a block for something that fits on one line.

---

## 3. Private header files (not exposed to developers)

Files under `Private/` follow `GorgeousObjectVariablePropertyTypeCustomization.h`:

1. **Wrap the whole file in `#if 0 ... #endif`** (`#if 0` then `#pragma once`; file ends with
   `#endif`). Keeps the code available for reference without compiling.
2. **No `//<===...===>` section banners and no `UCLASS` Help `meta` tags** (internal classes
   are not registered in Help).
3. **A descriptive `/* ... */` comment sits directly above the class** in prose.
4. **Member variables are grouped and readable.** Nested structs declared first, then private
   members in a short `private:` block. Single `//` comment per non-obvious member; do **not**
   wrap each member in a `/** ... */` block.
5. Public/private interface functions are listed plainly, no section dividers.

---

## 4. Enums (`UENUM`)

- **One-line purpose comment only** — no bulky descriptor. Use `/** ... */` with
  `@author Nils Bergemann`:

  ```cpp
  /**
   * Enumeration defining the kind of source data for a data schema mapping.
   *
   * @author Nils Bergemann
   */
  UENUM(BlueprintType)
  enum class EGorgeousDataSchemaSourceKind_E : uint8
  {
      Source_AnyUObject UMETA(DisplayName = "Any UObject Asset"),
      // ...
  };
  ```
- Enum suffix `_E`; values use `UMETA(DisplayName = "...")`.
- For help tooltips, add `meta = (ShortTooltip = "...")` on `UENUM` and `ToolTip = "..."` on
  each `UMETA` (see `GorgeousConditionalObjectChooserTypes.h`).

---

## 5. Structs (`USTRUCT`)

- **One concise `/** ... */`** describing the struct's purpose, optionally with `@author`:
  ```cpp
  /**
   * Structure for Source field definitions in a data schema mapping.
   *
   * @author Nils Bergemann
   */
  USTRUCT(BlueprintType)
  struct GORGEOUSCORERUNTIME_API FGorgeousDataSchemaSourceDefinition_S
  ```
- Struct suffix `_S`. Add the module API macro only for exported structs.
- Properties/constructors/operators inside use the same comment rules as class members (§2).

### `using` alias — RESTRICTED

Add a file-end `using` alias **ONLY** for classes that derive (directly or transitively) from
**`UGorgeous`** or from **`UBlueprintFunctionLibrary`**. Place it after the class's closing
`};`, at the very end of the header:

```cpp
using UGT_IF = UGorgeousInteractionFoundation;   // derives from UGorgeous
using UGT_UIFL = UGorgeousUIFoundationLibrary;    // derives from UBlueprintFunctionLibrary
```

- Prefix `UGT_`, suffixed with short readable initials.
- **Do NOT** add the alias for classes deriving from `UObject`, `UCommon*`, `AHUD`, any
  `USubsystem`, `UDeveloperSettings`, `UPrimaryDataAsset`, or a custom base that itself derives
  from `UObject` (e.g. a `UGorgeousUIProcessor : public UObject` gets **no** alias). Only the
  two named roots qualify.

---

## 6. Source files (`.cpp`)

Follow `GorgeousConditionalObjectChooser.cpp`:

1. **No trailing newline at EOF.** The last line of code is the final line.
2. **Banner** identical to the header banner (§1).
3. **Include** the matching header first, then others:
   ```cpp
   #include "GeneralSystems/CommonUIFoundation/GorgeousUIFoundationSubsystem.h"
   ```
4. **`// Helpers` banner** above any anonymous `namespace` or file-scope helpers:
   ```cpp
   //=============================================================================
   // Helpers
   //=============================================================================

   namespace { /* ... */ }
   ```
5. **`// UXXXX Implementation` banner** before the class's function definitions:
   ```cpp
   //=============================================================================
   // UGorgeousConditionalObjectChooser Implementation
   //=============================================================================
   ```
6. Brief `//` comments inside functions explaining non-obvious logic. No header-style banners
   inside individual functions.
7. If there is no helper namespace, omit the Helpers banner. If the cpp implements multiple
   classes, add one `// XXXX Implementation` banner per class.

---

## 7. Quick checklist

- [ ] Banner (correct year) + `#pragma once`; cpp keeps the banner too.
- [ ] Includes wrapped in the banner with NO blank line under the open divider or above the
      close divider; `.generated.h` last.
- [ ] Public `UCLASS` has Help `meta` tags (merged, not replacing existing specifiers) and a
      Class Info block immediately above it (no blank gap).
- [ ] Forward declarations, if any, wrapped in a `Forward Declarations` block.
- [ ] Only used section groups present (no empty groups).
- [ ] Every function `/** */` has `@param` for each param and `@return` only when non-void.
- [ ] `UPROPERTY`/members use single-line `//` unless the text truly overflows one line.
- [ ] Enums/structs have one-line purpose comments, not bulky descriptors.
- [ ] `using UGT_` alias present **only** for `UGorgeous` / `UBlueprintFunctionLibrary`
      subclasses.
- [ ] No comment placed outside its class/struct body; no blank line between a `UCLASS` and its
      comment block.
- [ ] `.cpp`: banner, includes, optional `// Helpers`, `// UXXXX Implementation`, no trailing
      newline.

---

## 8. Machine-readable schema (deterministic application spec)

This section is the **canonical, executable contract**. Every agent (human or AI) MUST transform
any Gorgeous `.h`/`.cpp` file into the structure defined here. The rules above are prose;
this section is the schema. Apply it top-to-bottom, do not improvise.

### 8.1 Tokens (verbatim strings — never paraphrase)

```
BANNER_COPYRIGHT = "// Copyright (c) 2026 Simsalabim Studios (Nils Bergemann). All rights reserved."
BANNER_BOX_OPEN  = "/*==========================================================================>"
BANNER_LINE_1    = "|               Gorgeous Core - Core functionality provider                 |"
BANNER_SEP       = "| ------------------------------------------------------------------------- |"
BANNER_LINE_2    = "|         Copyright (C) 2026 Gorgeous Things by Simsalabim Studios,         |"
BANNER_LINE_3    = "|              administrated by Epic Nova. All rights reserved.             |"
BANNER_LINE_4    = "|                    Epic Nova is an independent entity,                    |"
BANNER_LINE_5    = "|          that is not affiliated with Epic Games in any capacity.          |"
BANNER_BOX_CLOSE = "<==========================================================================*/"
PRAGMA_ONCE     = "#pragma once"

INCL_OPEN   = "//<=============================--- Includes ---=============================>"
INCL_MOD    = "//<--------------------------=== Module Includes ===------------------------->"
INCL_ENG    = "//<--------------------------=== Engine Includes ===------------------------->"
INCL_MISC   = "//--------------=== Third Party & Miscellaneous Includes ===-----------------"
INCL_CLOSE  = "//<-------------------------------------------------------------------------->"

CLASSINFO_OPEN  = "/*"
CLASSINFO_T1    = "<=============================--- Class Info ---============================>"
CLASSINFO_T2    = "<-----------------------------=== Quick Info ===---------------------------->"
CLASSINFO_Q_DISP= "| Display Name: "
CLASSINFO_Q_FUNC= "| Functional Name: "
CLASSINFO_Q_PAR = "| Parent Class: "
CLASSINFO_Q_SUF = "| Class Suffix: "
CLASSINFO_Q_AUT = "| Author: Nils Bergemann"
CLASSINFO_SEP   = "<--------------------------------------------------------------------------->"
CLASSINFO_T3    = "<--------------------------=== Class Description ===------------------------>"
CLASSINFO_T4    = "<==========================================================================>"
CLASSINFO_CLOSE = "*/"

GROUP_OVERRIDES = "//<============================--- Overrides ---============================>"
GROUP_BP        = "//<=======================--- Blueprint Functions ---=======================>"
GROUP_VARS      = "//<====================--- UAT/UBT Exposed Variables ---====================>"
GROUP_CPPONLY   = "//<============================--- C++ Only ---=============================>"
GROUP_VARIABLES = "//<============================--- Variables ---============================>"
GROUP_CALLBACKS = "//<============================--- Callbacks ---============================>"
GROUP_FRIENDS   = "//<===================--- Friend Classes ---================>"
GROUP_CLOSE     = "//<------------------------------------------------------------------------->"

SUB_IF_OVERRIDES = "//<----------------------=== Interface Overrides ===------------------------>"
SUB_IF_END       = "//<----------------------=== End Interface Overrides ===-------------------->"
SUB_ACTOR        = "//<----------------------=== Actor Overrides ===---------------------------->"
SUB_UOBJECT      = "//<----------------------=== UObject Overrides ===-------------------------->"

HELP_META = "meta = ("
DOC_OVERVIEW = "DocumentationOverview  = \"https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/<MODULE_PATH>/Overview\""
DOC_API      = "DocumentationAPI = \"https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/<MODULE_PATH>/<ClassName>\""
DOC_EXAMPLES = "DocumentationExamples = \"https://gorgeous.simsalabim.studio/docs/gorgeous-core/Runtime/<MODULE_PATH>/Examples/\""
```

### 8.2 Document model (the AST every header must reduce to)

```
Header := Banner, PRAGMA_ONCE, BLANK,
          Includes,                       // see 8.3
          [ BLANK, ForwardDecls ],        // optional, only if forward decls exist
          [ BLANK, ClassInfo ],           // optional, only for public UCLASS
          BLANK,
          ClassOrStructOrEnum+            // the actual UE types
          [ UsingAlias ]                  // optional, only UGorgeous / UBlueprintFunctionLibrary roots

BLANK := exactly one empty line

Includes := INCL_OPEN, INCL_MOD, ModuleIncludes,
            INCL_ENG, EngineIncludes,
            INCL_MISC, MiscIncludes,
            INCL_CLOSE
   where each "...Includes" is a contiguous run of `#include "..."` lines
   with NO blank lines between any of them (header→include, include→include,
   include→next-header all contiguous).

ClassInfo := CLASSINFO_OPEN, CLASSINFO_T1, CLASSINFO_T2,
             CLASSINFO_Q_DISP  <DisplayName>,
             CLASSINFO_Q_FUNC  <ClassName>,
             CLASSINFO_Q_PAR   <ParentClassName>,
             CLASSINFO_Q_SUF   <Suffix | "-">,
             CLASSINFO_Q_AUT,
             CLASSINFO_SEP,
             CLASSINFO_T3,
             <one-or-more "| " wrapped description rows, each <= 77 chars>,
             CLASSINFO_SEP,
             CLASSINFO_T4,
             CLASSINFO_CLOSE
   // CLASSINFO_CLOSE ('*/') is DIRECTLY followed by the UCLASS line, NO blank line.

Class := [ CLASSINFO ], UCLASS_SPEC, "class", API, ClassName, ": public", ParentList, "{",
          GENERATED_BODY,
          [ Constructor ],                 // 8.4
          [ OverridesGroup ],              // 8.5
          [ BlueprintFunctionsGroup ],     // 8.6
          [ ExposedVarsGroup ],            // 8.7
          [ OtherGroups ],                 // 8.8
          "};"
```

### 8.3 Includes — hard constraints

1. `INCL_OPEN` is the line immediately after `PRAGMA_ONCE, BLANK`.
2. Immediately after `INCL_OPEN`: `INCL_MOD` (no blank line).
3. After `INCL_MOD`: module/plugin `#include` lines, contiguous.
4. Next line: `INCL_ENG` (no blank line before it, no blank line after it).
5. After `INCL_ENG`: engine `#include` lines, contiguous.
6. Next line: `INCL_MISC` (no blank line).
7. After `INCL_MISC`: misc includes, **with the `.generated.h` as the LAST line**.
8. Next line: `INCL_CLOSE` (no blank line before it).
9. After `INCL_CLOSE`: exactly one `BLANK` line, then the next top-level block.
10. Violation pattern to always reject: a blank line between `INCL_MOD`/`INCL_ENG`/`INCL_MISC`
    and their first `#include`, OR a blank line after any `#include` inside the block. **There
    are zero blank lines inside the Includes block.**

### 8.4 Constructor / destructor placement

- They sit **directly after `GENERATED_BODY()`** (one `BLANK` line between `GENERATED_BODY()`
  and the constructor comment).
- They are **NOT** wrapped in any group banner.
- Constructor gets a single-line `//` comment describing special init behaviour.
- Example:
  ```
      GENERATED_BODY()

      // Initializes the overlay and sets up its Signal Bridge interface boilerplate.
      UGorgeousCommonOverlay(const FObjectInitializer& ObjectInitializer);
  ```

### 8.5 `Overrides` group (order is fixed)

```
    //<============================--- Overrides ---============================>
    #pragma region Overrides
    public:

    //<----------------------=== Interface Overrides ===------------------------>

    UE_UI_WIDGET_INTERFACE_BOILERPLATE()
    //<----------------------=== End Interface Overrides ===-------------------->

    protected:

    // <one-line comment>
    virtual void SynchronizeProperties() override;

    // <one-line comment>
    virtual void OnWidgetRebuilt() override;

    // <one-line comment>
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;
    //<------------------------------------------------------------------------->
    #pragma endregion Overrides
```
- Group opens with `public:`.
- Immediately after the group banner: `#pragma region Overrides` (one blank line between banner
  and `#pragma region` is allowed). The matching `#pragma endregion Overrides` sits **directly
  after** the closing `//<--------...--------->` divider (no blank line).
- The interface boilerplate macro goes **only** inside the `Interface Overrides` sub-group
  (opened by `SUB_IF_OVERRIDES`, closed by `SUB_IF_END`).
- Virtual overrides follow under `protected:` (still inside the same `Overrides` group).
- Access specifiers may switch inside the group; the group close `GROUP_CLOSE` is the final line.
- **No blank line between the last member (or sub-group close) and the closing divider.**

### 8.6 `Blueprint Functions` group

```
    //<=======================--- Blueprint Functions ---=======================>
    #pragma region Blueprint Functions
    public:

    /**
     * <present-tense behaviour description>.
     *
     * @param <Name> <description>
     * @return <only if non-void>
     */
    UFUNCTION(...)
    ReturnType FunctionName(...);
    //<------------------------------------------------------------------------->
    #pragma endregion Blueprint Functions
```

### 8.7 `UAT/UBT Exposed Variables` group

```
    //<====================--- UAT/UBT Exposed Variables ---====================>
    #pragma region UAT/UBT Exposed Variables
    public:

    // <one-line description>
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gorgeous UI")
    FGameplayTag BindingTag;
    //<------------------------------------------------------------------------->
    #pragma endregion UAT/UBT Exposed Variables
```
- Each `UPROPERTY` is preceded by a single-line `//` comment (never a `/** */` block for short
  descriptions).

### 8.8 Group ordering (top-to-bottom, mandatory)

```
Constructor (ungrouped)
Overrides            (public: → Interface Overrides sub-group + protected: virtuals)
Blueprint Functions  (public:)
UAT/UBT Exposed Vars (public:)
[Callbacks]          (as needed)
[C++ Only]           (as needed)
[Variables]          (as needed)
[Friend Classes]     (as needed)
```
- If a sibling header in the same `Widgets/` (or equivalent) directory already defines the
  ordering, **copy that ordering verbatim** — do not invent a different one.
- Only emit groups that contain members. Empty groups are defects.

### 8.8.1 Region wrapping & inter-group spacing (HARD)

- **Wrap every group** in `#pragma region <GroupName>` … `#pragma endregion <GroupName>`, placed
  as shown in §8.5–§8.7 (region open right after the group banner; region close right after the
  group's closing divider).
- **`#pragma region` is DIRECTLY followed by the access specifier — no blank line between them.**
  (`#pragma region Overrides` then `public:` next line.) A blank line between the group banner
  and `#pragma region` is allowed; never between `#pragma region` and the access specifier.
- **Two blank lines between two top-level groups.** The `#pragma endregion` of group A is followed
  by exactly two blank lines, then the `//<===...===>` banner of group B. Never one, never three.
- **Inside a group: single blank lines only** (between banner and access spec, between members,
  around sub-groups). Never two blank lines inside a group.
- **A group's closing divider `//<--------...--------->` sticks directly to whatever is above it**
  (last member or last sub-group close). No blank line before it. Same for sub-group closes.
- **Access specifiers are flush-left (no leading tab);** members keep their normal one-level indent.

### 8.9 Comment-style decision table (deterministic)

| Element | Rule |
|---|---|
| Function (any) | `/** ... */` with `@param` per param, `@return` iff non-`void` |
| Override / trivial fn | single-line `//` acceptable |
| `UPROPERTY` / plain member | single-line `//` |
| `UENUM` | `/** ... */` + `@author Nils Bergemann` |
| `USTRUCT` | `/** ... */` (+ optional `@author`) |
| Text fits one line | `// Text.` |
| Text overflows one line | `/**\n *\n */` block — never a block for one-liner |

### 8.10 EOF rule

- File ends with the last character of the final line (e.g. `};`). **No trailing `\n`.**
- The `BLANK` separators are interior only.

### 8.11 Canonical normalized output (this file is the reference of record)

The file `GorgeousCommonOverlay.h` (after full style application) is the **golden sample**.
When in doubt, diff against its emitted shape:
- banner → `#pragma once` → blank → includes (no internal blanks) → blank → Class Info
  (`*/` directly above `UCLASS`) → blank → class body with the §8.4–§8.8 ordering, **every group
  wrapped in `#pragma region`/`#pragma endregion`**, **exactly two blank lines between groups**,
  **no blank line before any group-close divider** → `};` with no trailing newline.

### 8.12 Transformation algorithm (pseudo-code an agent runs)

```
1. Preserve ALL code tokens: signatures, specifiers, macros, include paths, statements.
2. Rewrite banner to BANNER_* verbatim; set PRAGMA_ONCE.
3. Emit Includes per §8.3 (re-sort existing includes into Module/Engine/Misc; .generated.h last).
4. If public UCLASS: insert Class Info per §8.2 between Includes and UCLASS (no blank gap).
5. Merge Help meta into UCLASS spec (preserve existing specifiers); one doc tag per line.
6. Re-order class body members into groups per §8.8:
     a. constructor first (ungrouped)            §8.4
     b. Overrides group (Interface sub-group + protected virtuals)  §8.5
     c. Blueprint Functions group                §8.6
     d. Exposed Variables group                   §8.7
     e. remaining groups                          §8.8
7. Wrap every group in #pragma region/#pragma endregion (§8.8.1); insert access specifiers.
8. Comment every member per §8.9 (behaviour, not feature list).
9. Trim file to no trailing newline.
10. Verify: zero blank lines inside Includes; ClassInfo adjacent to UCLASS; groups only if
    non-empty; meta tags multi-line; each group region-wrapped; exactly two blank lines between
    groups; no blank line before any group-close divider; EOF has no trailing newline.
```
