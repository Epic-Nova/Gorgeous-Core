# Copyright (c) 2025-2026 Simsalabim Studios (Nils Bergemann). All rights reserved.
# ============================================================================
# GorgeousVault — Standalone CMake Build
# ============================================================================
#
# This CMakeLists.txt builds the GorgeousVault shared library (DLL/SO/DYLIB)
# as a STANDALONE project, completely outside of Unreal Build Tool.
#
# The Vault contains the real Object Variable and AutoReplication
# implementations. It is never distributed as source — only prebuilt
# binaries ship with the plugin.
#
# Usage:
#   cmake -B Build -S . -DUNREAL_ENGINE_DIR="D:/UnrealEngine/UE_5.4" -DCMAKE_BUILD_TYPE=Release
#   cmake --build Build --config Release
#
# For obfuscated builds (requires Hikari/OLLVM or commercial equivalent):
#   cmake -B Build -S . -DUNREAL_ENGINE_DIR="D:/UnrealEngine/UE_5.4" \
#         -DCMAKE_BUILD_TYPE=Release -DGVAULT_ENABLE_OBFUSCATION=ON
# ============================================================================

cmake_minimum_required(VERSION 3.20)
project(GorgeousVault VERSION 1.0.0 LANGUAGES CXX)

# ── Options ────────────────────────────────────────────────────────────────
option(GVAULT_ENABLE_OBFUSCATION "Enable LLVM obfuscation passes (requires Hikari/OLLVM)" OFF)
option(GVAULT_ENABLE_ANTI_TAMPER "Enable compile-time integrity hash embedding" ON)
option(GVAULT_ENABLE_ANTI_DEBUG  "Enable anti-debug measures (disabled in Editor)" ON)
option(GVAULT_STRIP_SYMBOLS     "Strip debug symbols from the output binary" ON)

# ── Unreal Engine paths ───────────────────────────────────────────────────
if(NOT DEFINED UNREAL_ENGINE_DIR)
    message(FATAL_ERROR "UNREAL_ENGINE_DIR must be set. Example: -DUNREAL_ENGINE_DIR=\"D:/UnrealEngine/UE_5.4\"")
endif()

set(UE_SOURCE_DIR "${UNREAL_ENGINE_DIR}/Engine/Source")
set(UE_INTERMEDIATE_DIR "${UNREAL_ENGINE_DIR}/Engine/Intermediate")

# ── Plugin paths (for including the open-source headers) ──────────────────
# Adjust this if your workspace layout differs.
get_filename_component(PLUGIN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(GORGEOUS_CORE_SOURCE "${PLUGIN_ROOT}/Source")

# ── C++ Standard ──────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Platform detection ────────────────────────────────────────────────────
if(WIN32)
    set(GVAULT_PLATFORM "Win64")
    set(GVAULT_LIB_PREFIX "")
    set(GVAULT_LIB_SUFFIX ".dll")
    set(GVAULT_IMPORT_SUFFIX ".lib")
elseif(APPLE)
    set(GVAULT_PLATFORM "Mac")
    set(GVAULT_LIB_PREFIX "lib")
    set(GVAULT_LIB_SUFFIX ".dylib")
    set(GVAULT_IMPORT_SUFFIX "")
else()
    set(GVAULT_PLATFORM "Linux")
    set(GVAULT_LIB_PREFIX "lib")
    set(GVAULT_LIB_SUFFIX ".so")
    set(GVAULT_IMPORT_SUFFIX "")
endif()

# ── Collect Vault source files ────────────────────────────────────────────
set(VAULT_SOURCES
    # ── Object Variables ──
    Source/ObjectVariables/GorgeousObjectVariable_Impl.cpp
    Source/ObjectVariables/GorgeousRootObjectVariable_Impl.cpp
    Source/ObjectVariables/GorgeousObjectVariableTrunk_Impl.cpp
    Source/ObjectVariables/GorgeousObjectVariableRegistry_GIS_Impl.cpp
    Source/ObjectVariables/NativeObjectVariableDefinitions_Impl.cpp
    Source/ObjectVariables/GorgeousObjectVariableCmdletHandler_Impl.cpp

    # ── AutoReplication ──
    Source/AutoReplication/GorgeousAutoReplicationMixin_Impl.cpp
    Source/AutoReplication/GorgeousAutoReplicationCoordinator_Impl.cpp
    Source/AutoReplication/GorgeousAutoReplicationGraph_Impl.cpp
    Source/AutoReplication/GorgeousAutoReplicationIrisBackend_Impl.cpp
    Source/AutoReplication/GorgeousAutoReplicationRPCTransporter_Impl.cpp
    Source/AutoReplication/GorgeousAutoReplicationRPCRelayComponent_Impl.cpp
    Source/AutoReplication/GorgeousAutoReplicationRPCRequestAsyncAction_Impl.cpp
    Source/AutoReplication/GorgeousAutoReplicationWorldSubsystem_Impl.cpp
    Source/AutoReplication/GorgeousAutoReplicationNetworkingTypes_Impl.cpp
    Source/AutoReplication/GorgeousAutoReplicationTypes_Impl.cpp
    Source/AutoReplication/GorgeousAutoReplicationStatsCollector_Impl.cpp
    Source/AutoReplication/GorgeousAutoReplicationSettings_Impl.cpp
    Source/AutoReplication/GorgeousReplicationBeacon_Impl.cpp
    Source/AutoReplication/ObjectVariables/GorgeousRPC_OV_Impl.cpp
    Source/AutoReplication/BlueprintFunctionLibraries/GorgeousAutoReplicationNetworkingLibrary_Impl.cpp
    Source/AutoReplication/BlueprintFunctionLibraries/GorgeousAutoReplicationRPCExamples_Impl.cpp
    Source/AutoReplication/BlueprintFunctionLibraries/GorgeousAutoReplicationRPCPayloadLibrary_Impl.cpp

    # ── License Helper (drawn from GorgeousCoreLicenseHelper) ──
    Source/LicenseHelper/GorgeousLicenseHelper_Impl.cpp

    # ── Protection ──
    Source/VaultAntiTamper.cpp
    Source/VaultLicense.cpp
    Source/VaultChallengeResponse.cpp
    Source/VaultEntryPoint.cpp
)

# ── Shared library target ─────────────────────────────────────────────────
add_library(GorgeousVault SHARED ${VAULT_SOURCES})

# Export macro: VAULT_API = dllexport when building, dllimport when consuming
target_compile_definitions(GorgeousVault PRIVATE
    GORGEOUS_VAULT_EXPORTS=1
    # UE compatibility defines
    UE_BUILD_SHIPPING=0
    UE_BUILD_TEST=0
    UE_BUILD_DEVELOPMENT=1
    UE_EDITOR=1
    # WITH_EDITOR=0: The Vault links against UnrealEditor-*.lib (hence UE_EDITOR=1)
    # but does NOT use editor UI features (toast notifications, message log delegate).
    # Setting this to 0 compiles out the only non-inline external symbol reference
    # (GetGorgeousLogEntryDelegate) so we don't need a GorgeousCoreRuntimeUtilities
    # import library — which UBT does not generate.
    WITH_EDITOR=0
    PLATFORM_WINDOWS=$<BOOL:${WIN32}>
    PLATFORM_LINUX=$<BOOL:${UNIX}>
    PLATFORM_MAC=$<BOOL:${APPLE}>
)

# ── Include directories ───────────────────────────────────────────────────
target_include_directories(GorgeousVault PRIVATE
    # Vault's own public API header
    ${CMAKE_CURRENT_SOURCE_DIR}/Include

    # ── GorgeousCore open-source headers ───────────────────────────────
    # GorgeousCoreRuntime
    ${GORGEOUS_CORE_SOURCE}/GorgeousCoreRuntime/Public
    ${GORGEOUS_CORE_SOURCE}/GorgeousCoreRuntime/Public/ModuleCore

    # GorgeousCoreRuntimeUtilities (for GT_I_LOG, helpers, etc.)
    ${GORGEOUS_CORE_SOURCE}/GorgeousCoreRuntimeUtilities/Public
    ${GORGEOUS_CORE_SOURCE}/GorgeousCoreRuntimeUtilities/Public/ModuleCore
    ${GORGEOUS_CORE_SOURCE}/GorgeousCoreRuntimeUtilities/Public/Helpers
    ${GORGEOUS_CORE_SOURCE}/GorgeousCoreRuntimeUtilities/Public/Helpers/Macros
    ${GORGEOUS_CORE_SOURCE}/GorgeousCoreRuntimeUtilities/Public/Libraries
    ${GORGEOUS_CORE_SOURCE}/GorgeousCoreRuntimeUtilities/Public/Templates

    # ── UHT generated headers (produced by UBT; must build the project first) ──
    ${PLUGIN_ROOT}/Intermediate/Build/${GVAULT_PLATFORM}/UnrealEditor/Inc/GorgeousCoreRuntimeUtilities/UHT
    ${PLUGIN_ROOT}/Intermediate/Build/${GVAULT_PLATFORM}/UnrealEditor/Inc/GorgeousCoreRuntime/UHT

    # ── Unreal Engine headers ──
    # Core
    ${UE_SOURCE_DIR}/Runtime/Core/Public
    ${UE_SOURCE_DIR}/Runtime/Core/Internal
    ${UE_SOURCE_DIR}/Runtime/TraceLog/Public
    
    # CoreUObject
    ${UE_SOURCE_DIR}/Runtime/CoreUObject/Public
    ${UE_SOURCE_DIR}/Runtime/CoreUObject/Internal

    # Engine
    ${UE_SOURCE_DIR}/Runtime/Engine/Public
    ${UE_SOURCE_DIR}/Runtime/Engine/Classes

    # Net / Replication
    ${UE_SOURCE_DIR}/Runtime/Net/Core/Public
    ${UE_SOURCE_DIR}/Runtime/NetCore/Public
    ${UE_SOURCE_DIR}/Runtime/Networking/Public
    ${UE_SOURCE_DIR}/Runtime/Online/HTTP/Public

    # Replication Graph
    ${UE_SOURCE_DIR}/Plugins/Runtime/ReplicationGraph/Source/Public

    # Iris (conditional)
    ${UE_SOURCE_DIR}/Plugins/Runtime/Iris/Source/Public

    # Slate / UMG (needed for some header deps)
    ${UE_SOURCE_DIR}/Runtime/Slate/Public
    ${UE_SOURCE_DIR}/Runtime/SlateCore/Public
    ${UE_SOURCE_DIR}/Runtime/UMG/Public
    ${UE_SOURCE_DIR}/Runtime/InputCore/Public

    # GameplayTags
    ${UE_SOURCE_DIR}/Runtime/GameplayTags/Public

    # DeveloperSettings
    ${UE_SOURCE_DIR}/Runtime/DeveloperSettings/Public

    # JSON
    ${UE_SOURCE_DIR}/Runtime/Json/Public
    ${UE_SOURCE_DIR}/Runtime/JsonUtilities/Public

    # Projects
    ${UE_SOURCE_DIR}/Runtime/Projects/Public

    # UE generated headers
    ${UE_INTERMEDIATE_DIR}/Build/Win64/UnrealEditor/Inc
)

# ── Link against UE import libraries ──────────────────────────────────────
# UE modules are themselves DLLs. We link against their import libraries
# so the Vault can call UObject, FProperty, TArray, etc. at runtime.
#
# The actual .lib/.so paths depend on your UE build configuration.
# These are resolved at build time via the UE intermediate directory.

if(WIN32)
    set(UE_LIB_DIR "${UNREAL_ENGINE_DIR}/Engine/Binaries/Win64")
    
    # Core UE modules the Vault needs to link against.
    # These are the import libraries for the UE editor DLLs.
    set(UE_LINK_LIBS
        # Note: Actual .lib filenames vary by UE version and config.
        # These are placeholders — the real names are discovered during
        # the first successful build against a specific UE installation.
        #
        # Example pattern: UnrealEditor-Core.lib, UnrealEditor-CoreUObject.lib, etc.
    )

    # Glob for UE import libraries (editor configuration)
    file(GLOB UE_CORE_LIBS "${UE_LIB_DIR}/UnrealEditor-Core.lib")
    file(GLOB UE_COREUOBJECT_LIBS "${UE_LIB_DIR}/UnrealEditor-CoreUObject.lib")
    file(GLOB UE_ENGINE_LIBS "${UE_LIB_DIR}/UnrealEditor-Engine.lib")
    file(GLOB UE_NETCORE_LIBS "${UE_LIB_DIR}/UnrealEditor-NetCore.lib")
    file(GLOB UE_REPLICATION_GRAPH_LIBS "${UE_LIB_DIR}/UnrealEditor-ReplicationGraph.lib")

    target_link_libraries(GorgeousVault PRIVATE
        ${UE_CORE_LIBS}
        ${UE_COREUOBJECT_LIBS}
        ${UE_ENGINE_LIBS}
        ${UE_NETCORE_LIBS}
        ${UE_REPLICATION_GRAPH_LIBS}
    )
elseif(UNIX AND NOT APPLE)
    set(UE_LIB_DIR "${UNREAL_ENGINE_DIR}/Engine/Binaries/Linux")
    # Linux: link against .so files directly
    # TODO: Populate with actual Linux UE shared library paths
elseif(APPLE)
    set(UE_LIB_DIR "${UNREAL_ENGINE_DIR}/Engine/Binaries/Mac")
    # macOS: link against .dylib files
    # TODO: Populate with actual macOS UE shared library paths
endif()

# ── Output configuration ──────────────────────────────────────────────────
# Multi-config generators (Visual Studio) ignore the plain *_OUTPUT_DIRECTORY
# and append a per-config subdirectory (e.g. Release/). Setting the per-config
# variants explicitly ensures the DLL lands directly in Prebuilt/<Platform>/.
set(GVAULT_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Prebuilt/${GVAULT_PLATFORM}")

set_target_properties(GorgeousVault PROPERTIES
    OUTPUT_NAME "GorgeousVault"
    PREFIX "${GVAULT_LIB_PREFIX}"
    SUFFIX "${GVAULT_LIB_SUFFIX}"

    # Single-config generators (Ninja, Makefiles)
    RUNTIME_OUTPUT_DIRECTORY "${GVAULT_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${GVAULT_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${GVAULT_OUTPUT_DIR}"

    # Multi-config generators (Visual Studio) — prevent config subdirectory
    RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${GVAULT_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${GVAULT_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${GVAULT_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     "${GVAULT_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG          "${GVAULT_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE        "${GVAULT_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${GVAULT_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL     "${GVAULT_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG          "${GVAULT_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE        "${GVAULT_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${GVAULT_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL     "${GVAULT_OUTPUT_DIR}"

    # Ensure C-linkage exports are visible
    C_VISIBILITY_PRESET default
    CXX_VISIBILITY_PRESET default
    VISIBILITY_INLINES_HIDDEN OFF
)

# ── Obfuscation passes (optional) ────────────────────────────────────────
if(GVAULT_ENABLE_OBFUSCATION)
    message(STATUS "GorgeousVault: LLVM obfuscation enabled")
    # These flags are for Hikari / OLLVM-compatible toolchains.
    # Adjust for your specific obfuscator.
    target_compile_options(GorgeousVault PRIVATE
        -mllvm -enable-cffobf        # Control flow flattening
        -mllvm -enable-bcfobf        # Bogus control flow
        -mllvm -enable-strcry        # String encryption
        -mllvm -enable-subobf        # Instruction substitution
        -mllvm -enable-splitobf      # Basic block splitting
    )
endif()

# ── Symbol stripping (Release) ────────────────────────────────────────────
if(GVAULT_STRIP_SYMBOLS AND CMAKE_BUILD_TYPE STREQUAL "Release")
    if(WIN32)
        # On Windows, simply don't generate a .pdb for distribution
        target_link_options(GorgeousVault PRIVATE /DEBUG:NONE)
    else()
        # On Unix, strip the binary
        add_custom_command(TARGET GorgeousVault POST_BUILD
            COMMAND ${CMAKE_STRIP} "$<TARGET_FILE:GorgeousVault>"
            COMMENT "Stripping symbols from GorgeousVault"
        )
    endif()
endif()

# ── Post-build: Integrity hash embedding ──────────────────────────────────
if(GVAULT_ENABLE_ANTI_TAMPER)
    # The integrity hash script reads the .text section of the built DLL,
    # computes SHA-256, and writes it into the reserved .gvault_meta section.
    #
    # This script is NOT included in the open-source distribution.
    # It lives in the CI pipeline or a separate private tools repository.
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Tools/embed_integrity_hash.py")
        add_custom_command(TARGET GorgeousVault POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Embedding integrity hash into GorgeousVault..."
            COMMAND python3 "${CMAKE_CURRENT_SOURCE_DIR}/Tools/embed_integrity_hash.py"
                    "$<TARGET_FILE:GorgeousVault>"
            COMMENT "Post-link: embedding .text section SHA-256 into .gvault_meta"
        )
    else()
        message(WARNING "GorgeousVault: Anti-tamper enabled but embed_integrity_hash.py not found. Skipping.")
    endif()
endif()

# ── Install target (copies to Prebuilt/) ──────────────────────────────────
install(TARGETS GorgeousVault
    RUNTIME DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/Prebuilt/${GVAULT_PLATFORM}"
    LIBRARY DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/Prebuilt/${GVAULT_PLATFORM}"
    ARCHIVE DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/Prebuilt/${GVAULT_PLATFORM}"
)

# ── Summary ───────────────────────────────────────────────────────────────
message(STATUS "=========================================================")
message(STATUS " GorgeousVault Configuration")
message(STATUS "=========================================================")
message(STATUS "  Platform:          ${GVAULT_PLATFORM}")
message(STATUS "  UE Engine Dir:     ${UNREAL_ENGINE_DIR}")
message(STATUS "  Plugin Root:       ${PLUGIN_ROOT}")
message(STATUS "  Obfuscation:       ${GVAULT_ENABLE_OBFUSCATION}")
message(STATUS "  Anti-Tamper:       ${GVAULT_ENABLE_ANTI_TAMPER}")
message(STATUS "  Anti-Debug:        ${GVAULT_ENABLE_ANTI_DEBUG}")
message(STATUS "  Strip Symbols:     ${GVAULT_STRIP_SYMBOLS}")
message(STATUS "  Output:            Prebuilt/${GVAULT_PLATFORM}/")
message(STATUS "=========================================================")
