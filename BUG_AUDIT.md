# Gorgeous-Core + Gorgeous-Inventory — Bug Audit

**Engine:** Unreal Engine 5.7 · **Scope:** `Gorgeous-Core` and `Gorgeous-Inventory` (all runtime + editor modules)
**Method:** 26 subsystem finders read every source file in their partition and reported concrete defects (not style). Each candidate was then handed to an independent *adversarial verifier* that re-read the code and tried to **refute** it (default-to-refuted, because the code cannot be compiled locally). Only findings a verifier could not refute are listed as verified.

> ⚠️ **Partial run.** The adversarial-verification pass was interrupted by a session token limit after ~25 of ~100 candidates were verified (10 survived, 13 refuted). The remaining finder candidates are listed in the *Unverified* section — they are raw finder output and have **not** been adversarially checked yet. Re-run the audit to verify them.

---

## 0. Fixed this session

### ✅ Attachment/World-Interaction editor crashes on "Open Editor" — `ensure(bTickable)` in `FDetailItemNode::Tick`
**File:** `Gorgeous-Inventory/Source/GorgeousInventoryEditor/Private/SourceFiles/Editor/SInventoryWorldInteractionWidget.cpp`
**Symptom:** Opening the World Interaction editor from the instanced adapter pack (the "Open Editor" button) fired `Ensure condition failed: bTickable [DetailItemNode.cpp:779]` from `SDetailsViewBase::Tick → FDetailLayoutBuilderImpl::Tick`.

**Root cause:** The widget reused a single `FStructOnScope` for `FGorgeousItemEquipAttachmentKey_S` and **overwrote its memory in place** on every socket/mesh selection, then asked the *live* `IStructureDetailsView` to refresh over the same pointer. The struct has many `EditConditionHides` properties and several `TArray`/`TMap` members, all of which become *tickable* detail nodes. Refreshing in place left the details view's `FDetailLayoutBuilderImpl` ticking a half-rebuilt `TickableNodes` list. The earlier `SetCanTick(false/true)` gate only narrowed the window; it did not remove it.

**Fix:** Never mutate/refresh the live view's data. The selected value is staged into `PendingStructValue`; on the next `Tick` a **brand-new `FStructOnScope` and a brand-new `IStructureDetailsView`** are built and swapped into a host `SBox` (`StructDetailsContainer`). A freshly constructed detail layout always has a self-consistent `TickableNodes` list, so the ensure cannot fire. Array "Add (+)" and `EditConditionHides` keep working because the new view ticks its own consistent layout normally. **The "Open Editor" button is unchanged.**

---

## 1. Verified findings (survived adversarial verification)

Ranked by corrected severity. `CONFIRMED` = a concrete wrong-outcome path was traced in current code. `PLAUSIBLE` = real defect whose trigger depends on caller/engine context that can't be fully pinned from source.

### 1.1 — HIGH · CONFIRMED · Loop bound clobbered mid-iteration (editor crash / silent truncation)
**File:** `Gorgeous-Core/Source/GorgeousCoreEditor/Private/SourceFiles/PropertyTypeCustomizations/GorgeousFunctionalStructurePropertyTypeCustomization.cpp:72`
The outer `for (uint32 i = 0; i < NumChildren; i++)` bound is **overwritten** inside the loop by `ChildHandle->GetNumChildren(NumChildren)` in the `ShowOnlyInnerProperties` branch. `GetNumChildren` writes (not accumulates) its out-param, so the outer bound becomes the grandchild count.
- If grandchildren < parent children → outer loop ends early: remaining direct children are dropped from the panel **and never get their Pre/PostEditChange delegates bound** (silent — edits stop firing `PostEditChangeProperty`).
- If grandchildren > parent children → `GetChildHandle(i).ToSharedRef()` is called out of range → null `ToSharedRef()` asserts → **editor crash**.
**Fix:** use a separate local for the grandchild count.

### 1.2 — MEDIUM · CONFIRMED · Null-deref crash in stale resource-guard sweep at startup
**File:** `Gorgeous-Core/Source/GorgeousCoreEditorUtilities/Private/SourceFiles/ExtensionResourceGuard/GorgeousExtensionResourceGuard.cpp:224`
`IPluginManager::Get().FindPlugin(Guard->OwningPluginName.ToString())->GetDescriptor()` is dereferenced with **no `IsValid()` check** — the four sibling call sites in the same file all check. A content-less guard (`bIsContentPackGuard=false`) whose `OwningPluginName` isn't resolvable (renamed/removed plugin) reaches this line via `ReconcilePluginDependencies` and null-derefs during editor-startup reconciliation.
**Fix:** capture the `TSharedPtr<IPlugin>` into a local, `IsValid()`-check, `continue` if invalid (matches lines 257-261).

### 1.3 — MEDIUM · CONFIRMED · GameplayTag chooser resolves FIRST/LAST/RANDOM from the wrong tag set
**File:** `Gorgeous-Core/Source/GorgeousCoreRuntime/Private/SourceFiles/ConditionalObjectChoosers/Conditions/GorgeousGameplayTagCondition.cpp:40`
`FoundTags.Append(Pair.Key.Container.GetGameplayTagArray())` fills the candidate list from the **mapping keys**, not from the source container returned by `GetGameplayTagContainer()`. The enum tooltips document "…found from the gameplay tag container" (the source). With multi-tag keys or multiple entries, FIRST/LAST/RANDOM can return a mapping entry that has **zero overlap** with the actual source container. Single-tag keys coincidentally work.
**Fix:** build `FoundTags` from `GetGameplayTagContainer().GetGameplayTagArray()` filtered to tags present in matching keys.

### 1.4 — MEDIUM · PLAUSIBLE · TMap key contract violated (order-dependent hash vs set-like equality)
**File:** `Gorgeous-Core/Source/GorgeousCoreRuntime/Public/ConditionalObjectChoosers/GorgeousConditionalObjectChooserStructures.h:69`
`GetTypeHash(FGameplayTagContainerWrapper_S)` folds per-tag hashes with the **non-commutative** `HashCombine`, so the hash is order-dependent; `operator==` delegates to `FGameplayTagContainer::operator==` which is **set-like** (order-independent). Two equal keys can hash differently → `TMap::Find` misses. Only the `Find`-based `FindConditionMappingForTagContainer` path is at risk (the main `CheckCondition` path iterates linearly); single-tag keys can't reorder.
**Fix:** accumulate the hash commutatively (`Hash += GetTypeHash(Tag)` or XOR).

### 1.5 — MEDIUM · PLAUSIBLE · Dangling raw struct pointer in property customization
**File:** `Gorgeous-Core/Source/GorgeousCoreEditor/Private/SourceFiles/PropertyTypeCustomizations/GorgeousFunctionalStructurePropertyTypeCustomization.cpp:123`
A raw `FGorgeousFunctionalStructure_S*` is cached from `GetValueData` and later dereferenced by SP-bound Pre/PostEditChange delegates; the guards only null-check, never revalidate. If the owning `TArray` reallocates or the object is reinstanced (BP recompile), the pointer dangles → potential use-after-free during interactive editing. (Depends on engine refresh/delegate ordering, hence PLAUSIBLE.)
**Fix:** re-resolve via a stored `TWeakPtr<IPropertyHandle>`+`GetValueData` inside each delegate instead of caching.

### 1.6 — MEDIUM · PLAUSIBLE · Missing null-check after `InstantiateHandler`
**File:** `Gorgeous-Core/Source/GorgeousCoreEditorUtilities/Private/SourceFiles/Libraries/GorgeousEditorLoggingBlueprintFunctionLibrary.cpp:66`
`InstantiateHandler` is documented to return `nullptr` on failure, but `ExecuteHyperlinkAction` immediately does `HandlerObject->GetClass()->FindFunctionByName(...)`. Clicking a log hyperlink whose handler class is abstract/GC'd can null-deref. (Whether `NewObject` returns null vs. asserts for an abstract class is engine-dependent, hence PLAUSIBLE — but the missing check is real.)
**Fix:** `if (!HandlerObject) { GT_E_LOG(...); return; }`.

### 1.7 — MEDIUM · PLAUSIBLE · `PostLoad` re-duplicates a valid instanced `ConditionCheck`
**File:** `Gorgeous-Core/Source/GorgeousCoreRuntime/Private/SourceFiles/ConditionalObjectChoosers/GorgeousConditionalObjectChooser.cpp:96`
`PostLoad` funnels the loaded instanced `ConditionCheck` back through `SetConditionCheck`, whose `IsTemplate() || IsLikelyPlaceholder() || GetOuter()!=this` branch `DuplicateObject`s it. For a `Blueprintable` subclass CDO/archetype, `IsTemplate()` is true, so the serialized subobject is replaced by a fresh duplicate on every CDO load (name churn / broken archetype-by-name references). Common non-template instances are unaffected.
**Fix:** in `PostLoad`, only sanitize placeholders; don't re-duplicate a correctly-outered instanced subobject.

### 1.8 — LOW · PLAUSIBLE · Boolean/IsValid condition error path collides with valid index 0
**File:** `Gorgeous-Core/Source/GorgeousCoreRuntime/Private/SourceFiles/ConditionalObjectChoosers/Conditions/GorgeousBooleanCondition.cpp:47`
`CheckCondition_Implementation` returns `uint8` used as an index; the `default:` (invalid Mode) returns `false` (=0), indistinguishable from a legitimate "condition false → index 0". A corrupted Mode silently selects `Conditions[0]` instead of failing safe. Unreachable from normal enum use (contiguous 0-8), so it needs data corruption / enum reorder.
**Fix:** return an out-of-range sentinel so `IsValidIndex` yields `nullptr`.

### 1.9 — LOW · PLAUSIBLE · Statistics maps never purge GC'd weak keys
**File:** `Gorgeous-Core/Source/GorgeousCoreRuntime/Private/SourceFiles/ModuleCore/Interfaces/GorgeousStatisticsInterface.cpp:82`
`TrackedObjects`/`ActiveSamples`/`LatestSnapshots` are keyed on `TWeakObjectPtr<UObject>`; the snapshot paths only `continue` on invalid keys, never erase them. Objects GC'd without an explicit `Remove/EndSample` leave stale keys forever → slow unbounded growth + wasted iteration. (The finder's "collision with a reused slot" claim was refuted — the serial number prevents that — so it's growth-only.)
**Fix:** compact invalid weak keys during `GatherCurrentHeapSnapshot`.

### 1.10 — LOW · PLAUSIBLE · Reference bound to a null `FAutomationTestBase` (latent UB)
**File:** `Gorgeous-Core/Source/GorgeousCoreRuntime/Private/UnitTests/Helpers/GorgeousMultiClientTestHarness.cpp:115`
`FGorgeousInsightScenarioContext Ctx(... OptionalTest ? *OptionalTest : *static_cast<FAutomationTestBase*>(nullptr) ...)` binds a reference member to a dereferenced null pointer whenever `OptionalTest` is null (which `RunTests` always passes). Standard-level UB at construction, but **no current runner reads `Context.Test`**, so it never dereferences → benign today. Worth fixing before something does read it.
**Fix:** make the context take `FAutomationTestBase*` (nullable pointer), not a reference.

---

## 2. Unverified candidates (verification cut off by session limit)

Raw finder output — **not yet adversarially verified.** Each is the strongest single finding from its subsystem finder; most finders also reported additional lower-ranked items. Treat severities as the finder's claim. Re-run the audit to verify. Items marked 🔎 touch files we've edited recently and are the highest-value re-checks.

| # | Sev (claimed) | File:line | Claim |
|---|---|---|---|
| A | HIGH ×2 (corroborated by two finders) | `GorgeousCoreRuntime/.../AutoReplication/BlueprintFunctionLibraries/GorgeousAutoReplicationRPCPayloadLibrary.cpp:94 & :225` | `FName` RPC arguments serialized as raw name-table bytes (`memcpy` of process-local `FName`) → corrupt/garbage names across the network. |
| B | CRITICAL | `GorgeousInventoryRuntime/.../Components/GorgeousInventoryManager_AC.cpp:34` | `CachedGlobalPhaseHooks`/`CachedGlobalEffectContracts` are file-scope **static** `TObjectPtr` arrays with no UPROPERTY rooting → GC can collect them → use-after-free. |
| C | CRITICAL | `GorgeousInventoryRuntime/.../InventoryExtensions/Adapters/WorldInteraction/GorgeousInventoryWorldInteractionLibrary.cpp:236` | `ExecuteEquippedItemSecondary` indexes `InteractionActions[2]` with no bounds check; default item has fewer → out-of-bounds crash. |
| D | CRITICAL | `GorgeousInventoryRuntime/.../InventoryExtensions/Contracts/GorgeousInventoryContract_NestedGate.cpp:32` | Guards `if (ItemData.ItemAsset)` (soft-ptr *set* check) then dereferences an **unloaded** `TSoftObjectPtr` → null crash. |
| E | CRITICAL | `GorgeousInventoryRuntime/Private/UnitTests/GorgeousInventoryKernelTests.cpp:36` | Scenario runners call `Context.WorldContextObject->GetWorld()` with no null check; `RunTest` can pass a null world. |
| F | HIGH | `GorgeousCoreRuntime/Public/ObjectVariables/GorgeousObjectVariable.h:644` | `SetDynamicProperty/GetDynamicProperty` gate `FObjectProperty` handling on `std::is_base_of_v<UObject, InTCppType>`, but `InTCppType` is a **pointer** type → always false → instanced dynamic `UObject*` properties silently ignored. |
| G | HIGH | `GorgeousCoreRuntime/.../InsightMatrix/Slate/GorgeousInsightDebugPanel.cpp:1589` | Per-row button handler calls `TestsListView->RebuildList()`, destroying the live widget mid-click → use-after-free. |
| H | HIGH 🔎 | `GorgeousInventoryRuntime/Public/ModuleCore/GorgeousInventoryRuntimeStructures.h:311` | `ResolveEquipOffset` can't find offsets saved under **multi-socket keys**: identity (`operator==`/`GetTypeHash`) uses the raw `SocketName` string verbatim while the rest of the subsystem treats it as a comma-joined list. *(Directly concerns the multi-socket feature we added.)* |
| I | HIGH 🔎 | `GorgeousInventoryEditor/.../Editor/SInventoryWorldInteractionWidget.cpp:316` | `OnMeshSelectionChanged` resolves `SelectedMeshIndex` by matching the display **asset name** string; two meshes with the same asset name (different folders) collapse to the wrong index → edit/save the wrong mesh. *(In the file we just fixed.)* |
| J | HIGH 🔎 | `GorgeousInventoryEditor/.../Editor/InventoryPreviewViewport.cpp:479` | `FInventoryPreviewViewportClient` holds raw `UObject*` (`SpawnedItemActor`, `PreviewSkelComp`, `PendingMesh`) but never overrides `FGCObject::AddReferencedObjects` → GC can free them mid-use. *(In our gizmo file.)* |
| K | HIGH | `GorgeousInventoryRuntime/.../Actors/GorgeousItem.cpp:406` | `UpdateRefreshNeed` dereferences a `TArray::FindByPredicate` result with no null check. |
| L | HIGH | `GorgeousInventoryRuntime/.../CommonUIFoundation/Widgets/InventoryTooltip_GUW.cpp:23` | `SetItemData` calls `ItemData.ItemAsset.Get()->DisplayName` on an unloaded soft-ptr → null crash. |
| M | HIGH | `GorgeousInventoryRuntime/.../GorgeousInventoryRuntimeModule.cpp:27` | Inventory Insight provider is created but never registered with the subsystem (and cast to the wrong type). |
| N | HIGH | `GorgeousInventoryEditor/.../GorgeousInventoryEditorModule.cpp:59` | The "Object" property-type customization is gated to the base `UGorgeousInventoryAdapterPack`, hiding **all** properties of other subclasses (e.g. CommonUIFoundation adapter). |
| O | MEDIUM | `GorgeousCoreRuntime/.../DataSchemaMapping/GorgeousDataSchemaMapper.cpp:311` | `Transform_MultiplyNumeric`/`Transform_AddNumericOffset` format results with `FString::SanitizeFloat`, producing a decimal string that fails to import into integer/byte target properties. |
| P | MEDIUM | `GorgeousCoreRuntime/.../GorgeousCoreRuntimeModule.cpp:100` | `FGorgeousCoreInsightMatrixProvider` created with `new` is never `delete`d on shutdown (pointer only null-ed) → leak. |
| Q | MEDIUM | `GorgeousCoreRuntime/.../QualityOfLife/GorgeousPlayerState.cpp:122` | `Server_SetGorgeousStablePlayerId` commits the replicated ID even when the registry rejects it as a duplicate (ignores the `bool` result). |
| R | MEDIUM | `GorgeousInventoryRuntime/.../PhaseHook/GorgeousInventoryPhaseHook_Cooldown.cpp:42` | Cooldown timestamp is stamped during `ValidateInput`, before later hooks can block the action → cooldown starts for actions that never execute. |

**Refuted (13):** raw-`FName` concerns aside, verifiers rejected e.g. a style-set `FindPlugin` deref (single safe expansion), asset-type-action weak-ptr null (by-design), object-variable registry GC (registry holds strong refs), several DataSchemaMapping GC-during-preview claims (ordering makes GC unreachable), and a networking "local write" path (intentional). These are *not* bugs.

---

## 3. Recommended order of work
1. **Verify & fix the CRITICALs (B, C, D, E)** — GC use-after-free and out-of-bounds/null crashes in the inventory runtime.
2. **H, I, J** — they sit in the equip/attachment editor code we're actively changing (multi-socket resolve, duplicate mesh-name index, viewport GC rooting).
3. **A** — networking `FName` corruption is high-impact if RPCs carry name args; corroborated by two independent finders.
4. Verified §1 items 1.1 / 1.2 (confirmed editor crashes) next.
5. Re-run the audit workflow after the token limit resets to adversarially verify §2 before acting on the lower-severity ones.
