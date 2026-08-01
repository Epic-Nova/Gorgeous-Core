#!/usr/bin/env bash
# ============================================================================
# GorgeousVault — Linux / macOS Build Script
# ============================================================================
#
# Builds the GorgeousVault shared library (.so / .dylib) using CMake.
#
# Usage:
#   ./BuildVault_Unix.sh
#   ./BuildVault_Unix.sh --ue-dir /opt/UnrealEngine/UE_5.4
#   ./BuildVault_Unix.sh --ue-dir /opt/UnrealEngine/UE_5.4 --config Release --obfuscation
#
# Prerequisites:
#   - CMake 3.20+
#   - Clang 14+ or GCC 12+ (C++20 support required)
#   - Unreal Engine 5.4+ source build
#
# ============================================================================

set -euo pipefail

# ── Defaults ──────────────────────────────────────────────────────────────
UE_DIR=""
CONFIG="Release"
OBFUSCATION=OFF
ANTI_TAMPER=ON
STRIP_SYMBOLS=ON
CLEAN=false
GENERATOR=""

# ── Parse arguments ───────────────────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case "$1" in
        --ue-dir)       UE_DIR="$2";        shift 2 ;;
        --config)       CONFIG="$2";        shift 2 ;;
        --obfuscation)  OBFUSCATION=ON;     shift ;;
        --no-anti-tamper) ANTI_TAMPER=OFF;  shift ;;
        --keep-symbols) STRIP_SYMBOLS=OFF;  shift ;;
        --clean)        CLEAN=true;         shift ;;
        --generator)    GENERATOR="$2";     shift 2 ;;
        -h|--help)
            echo "Usage: $0 [--ue-dir <path>] [--config Release|Debug] [--obfuscation] [--no-anti-tamper] [--keep-symbols] [--clean] [--generator <name>]"
            exit 0
            ;;
        *) echo "Unknown argument: $1"; exit 1 ;;
    esac
done

# ── Resolve paths ─────────────────────────────────────────────────────────
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VAULT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Detect platform
if [[ "$(uname)" == "Darwin" ]]; then
    PLATFORM="Mac"
    LIB_NAME="libGorgeousVault.dylib"
else
    PLATFORM="Linux"
    LIB_NAME="libGorgeousVault.so"
fi

BUILD_DIR="$VAULT_ROOT/Build/$PLATFORM"
PREBUILT_DIR="$VAULT_ROOT/Prebuilt/$PLATFORM"

echo "=========================================================="
echo " GorgeousVault — $PLATFORM Build"
echo "=========================================================="
echo "  Vault Root:  $VAULT_ROOT"
echo "  Build Dir:   $BUILD_DIR"
echo "  Config:      $CONFIG"

# ── Auto-detect UE directory ──────────────────────────────────────────────
if [[ -z "$UE_DIR" ]]; then
    # Try common locations
    COMMON_PATHS=(
        "/opt/UnrealEngine/UE_5.4"
        "$HOME/UnrealEngine/UE_5.4"
        "/usr/local/UnrealEngine/UE_5.4"
        "$HOME/UE_5.4"
    )
    for P in "${COMMON_PATHS[@]}"; do
        if [[ -d "$P/Engine/Source/Runtime/Core/Public" ]]; then
            UE_DIR="$P"
            break
        fi
    done

    if [[ -z "$UE_DIR" ]]; then
        echo "ERROR: Cannot auto-detect Unreal Engine directory. Pass --ue-dir explicitly."
        exit 1
    fi
fi

# Validate
if [[ ! -d "$UE_DIR/Engine/Source/Runtime/Core/Public" ]]; then
    echo "ERROR: Invalid Unreal Engine directory: $UE_DIR"
    exit 1
fi

echo "  UE Dir:      $UE_DIR"

# ── Check prerequisites ───────────────────────────────────────────────────
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found. Install CMake 3.20+."
    exit 1
fi

echo "  CMake:       $(cmake --version | head -n1)"

# Check for a C++20-capable compiler
if command -v clang++ &> /dev/null; then
    echo "  Compiler:    $(clang++ --version | head -n1)"
elif command -v g++ &> /dev/null; then
    echo "  Compiler:    $(g++ --version | head -n1)"
else
    echo "WARNING: No C++ compiler found on PATH. CMake will attempt to find one."
fi

# ── Clean if requested ────────────────────────────────────────────────────
if [[ "$CLEAN" == true && -d "$BUILD_DIR" ]]; then
    echo ""
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# ── Create build directory ────────────────────────────────────────────────
mkdir -p "$BUILD_DIR"

# ── Build CMake arguments ─────────────────────────────────────────────────
CMAKE_ARGS=(
    -S "$VAULT_ROOT"
    -B "$BUILD_DIR"
    -DUNREAL_ENGINE_DIR="$UE_DIR"
    -DCMAKE_BUILD_TYPE="$CONFIG"
    -DGVAULT_ENABLE_OBFUSCATION="$OBFUSCATION"
    -DGVAULT_ENABLE_ANTI_TAMPER="$ANTI_TAMPER"
    -DGVAULT_STRIP_SYMBOLS="$STRIP_SYMBOLS"
)

# Generator selection
if [[ -n "$GENERATOR" ]]; then
    CMAKE_ARGS+=(-G "$GENERATOR")
elif command -v ninja &> /dev/null; then
    CMAKE_ARGS+=(-G "Ninja")
    echo "  Generator:   Ninja"
else
    CMAKE_ARGS+=(-G "Unix Makefiles")
    echo "  Generator:   Unix Makefiles"
fi

# macOS: build universal binary (arm64 + x86_64) if requested
if [[ "$PLATFORM" == "Mac" ]]; then
    # Default to arm64 (Apple Silicon)
    CMAKE_ARGS+=(-DCMAKE_OSX_ARCHITECTURES="arm64")
    echo "  Architecture: arm64"
fi

echo "=========================================================="

# ── Configure ─────────────────────────────────────────────────────────────
echo ""
echo "[1/2] Configuring CMake..."
cmake "${CMAKE_ARGS[@]}"

# ── Build ─────────────────────────────────────────────────────────────────
NPROC=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
echo ""
echo "[2/2] Building GorgeousVault ($CONFIG) with $NPROC parallel jobs..."
cmake --build "$BUILD_DIR" --config "$CONFIG" --parallel "$NPROC"

# ── Verify output ─────────────────────────────────────────────────────────
LIB_PATH="$PREBUILT_DIR/$LIB_NAME"

if [[ -f "$LIB_PATH" ]]; then
    SIZE=$(du -h "$LIB_PATH" | cut -f1)
    echo ""
    echo "=========================================================="
    echo " BUILD SUCCEEDED"
    echo "=========================================================="
    echo "  Output: $LIB_PATH"
    echo "  Size:   $SIZE"
    echo "=========================================================="
else
    echo ""
    echo "WARNING: Build completed but $LIB_NAME was not found at expected location."
    echo "Expected: $LIB_PATH"
    echo ""
    echo "Files in build directory:"
    find "$BUILD_DIR" -name "GorgeousVault*" -o -name "libGorgeousVault*" 2>/dev/null || true
fi
