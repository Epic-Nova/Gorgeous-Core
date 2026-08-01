# ============================================================================
# GorgeousVault — Win64 Build Script (PowerShell)
# ============================================================================
#
# Builds the GorgeousVault DLL for Windows 64-bit using CMake.
#
# Usage:
#   .\BuildVault_Win64.ps1
#   .\BuildVault_Win64.ps1 -UEDir "D:\UnrealEngine\UE_5.4"
#   .\BuildVault_Win64.ps1 -UEDir "D:\UnrealEngine\UE_5.4" -Config Release -Obfuscation
#
# Prerequisites:
#   - CMake 3.20+ on PATH
#   - Visual Studio 2022 with C++ desktop workload (MSVC toolchain)
#   - Unreal Engine 5.4+ source build (for import libraries)
#
# ============================================================================

[CmdletBinding()]
param(
    # Path to Unreal Engine root (containing Engine/ folder)
    [Parameter()]
    [string]$UEDir = "",

    # Build configuration: Release or Debug
    [Parameter()]
    [ValidateSet("Release", "Debug", "RelWithDebInfo")]
    [string]$Config = "Release",

    # Enable LLVM obfuscation (requires Hikari/OLLVM toolchain)
    [switch]$Obfuscation,

    # Disable anti-tamper integrity embedding
    [switch]$NoAntiTamper,

    # Disable symbol stripping
    [switch]$KeepSymbols,

    # Clean build directory before building
    [switch]$Clean,

    # CMake generator override (default: auto-detect)
    [string]$Generator = ""
)

$ErrorActionPreference = "Stop"

# ── Resolve paths ─────────────────────────────────────────────────────────
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$VaultRoot = Split-Path -Parent $ScriptDir
$BuildDir = Join-Path $VaultRoot "Build\Win64"
$PrebuiltDir = Join-Path $VaultRoot "Prebuilt\Win64"

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host " GorgeousVault — Win64 Build" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "  Vault Root:  $VaultRoot"
Write-Host "  Build Dir:   $BuildDir"
Write-Host "  Config:      $Config"

# ── Auto-detect UE directory ──────────────────────────────────────────────
if ([string]::IsNullOrWhiteSpace($UEDir)) {
    # Try to find UE from the .uproject file
    $UProjectPath = Resolve-Path (Join-Path $VaultRoot "..\..\..\..\..\GorgeousThings.uproject") -ErrorAction SilentlyContinue
    if ($UProjectPath) {
        # Check for common UE locations
        $CommonPaths = @(
            "D:\UnrealEngine\UE_5.4",
            "C:\UnrealEngine\UE_5.4",
            "C:\Program Files\Epic Games\UE_5.4",
            "$env:USERPROFILE\UnrealEngine\UE_5.4"
        )
        foreach ($Path in $CommonPaths) {
            if (Test-Path (Join-Path $Path "Engine\Build\BatchFiles\Build.bat")) {
                $UEDir = $Path
                break
            }
        }
    }

    if ([string]::IsNullOrWhiteSpace($UEDir)) {
        Write-Error "Cannot auto-detect Unreal Engine directory. Pass -UEDir explicitly."
        exit 1
    }
}

# Validate UE dir
if (-not (Test-Path (Join-Path $UEDir "Engine\Source\Runtime\Core\Public"))) {
    Write-Error "Invalid Unreal Engine directory: $UEDir (missing Engine\Source\Runtime\Core\Public)"
    exit 1
}

Write-Host "  UE Dir:      $UEDir"

# ── Check prerequisites ───────────────────────────────────────────────────
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) {
    Write-Error "CMake not found on PATH. Install CMake 3.20+ and ensure it's on PATH."
    exit 1
}

$cmakeVersion = & cmake --version | Select-Object -First 1
Write-Host "  CMake:       $cmakeVersion"

# ── Clean if requested ────────────────────────────────────────────────────
if ($Clean) {
    foreach ($Dir in @($BuildDir, $PrebuiltDir)) {
        if (Test-Path $Dir) {
            Write-Host "  Removing $Dir" -ForegroundColor Yellow
            Remove-Item -Recurse -Force $Dir
        }
    }
    Write-Host "  Clean complete.`n" -ForegroundColor Yellow
}

# ── Create build directory ────────────────────────────────────────────────
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
}

# ── Build CMake arguments ─────────────────────────────────────────────────
$cmakeArgs = @(
    "-S", $VaultRoot,
    "-B", $BuildDir,
    "-DUNREAL_ENGINE_DIR=`"$UEDir`"",
    "-DCMAKE_BUILD_TYPE=$Config"
)

# Generator selection
if (-not [string]::IsNullOrWhiteSpace($Generator)) {
    $cmakeArgs += @("-G", "`"$Generator`"")
} else {
    # Prefer Visual Studio 2022 if available, fall back to Ninja
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vsWhere) {
        $vsPath = & $vsWhere -latest -property installationPath 2>$null
        if ($vsPath) {
            $cmakeArgs += @("-G", "Visual Studio 17 2022", "-A", "x64")
            Write-Host "  Generator:   Visual Studio 17 2022"
        }
    }
    
    $ninja = Get-Command ninja -ErrorAction SilentlyContinue
    if (-not ($cmakeArgs -contains "Visual Studio 17 2022") -and $ninja) {
        $cmakeArgs += @("-G", "Ninja")
        Write-Host "  Generator:   Ninja"
    }
}

# Optional flags
if ($Obfuscation) {
    $cmakeArgs += "-DGVAULT_ENABLE_OBFUSCATION=ON"
    Write-Host "  Obfuscation: ON" -ForegroundColor Yellow
}

if ($NoAntiTamper) {
    $cmakeArgs += "-DGVAULT_ENABLE_ANTI_TAMPER=OFF"
}

if ($KeepSymbols) {
    $cmakeArgs += "-DGVAULT_STRIP_SYMBOLS=OFF"
}

Write-Host "==========================================================" -ForegroundColor Cyan

# ── Configure ─────────────────────────────────────────────────────────────
Write-Host "`n[1/2] Configuring CMake..." -ForegroundColor Green
& cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

# ── Build ─────────────────────────────────────────────────────────────────
Write-Host "`n[2/2] Building GorgeousVault ($Config)..." -ForegroundColor Green
& cmake --build $BuildDir --config $Config --parallel
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

# ── Verify output ─────────────────────────────────────────────────────────
$DllPath = Join-Path $PrebuiltDir "GorgeousVault.dll"

# Multi-config generators (Visual Studio) may place output in a config subdirectory.
# Check both Prebuilt/Win64/<Config>/ and Build/Win64/<Config>/ as fallbacks.
if (-not (Test-Path $DllPath)) {
    $FallbackPaths = @(
        (Join-Path $PrebuiltDir "$Config\GorgeousVault.dll"),
        (Join-Path $BuildDir "$Config\GorgeousVault.dll")
    )
    foreach ($AltPath in $FallbackPaths) {
        if (Test-Path $AltPath) {
            if (-not (Test-Path $PrebuiltDir)) {
                New-Item -ItemType Directory -Force -Path $PrebuiltDir | Out-Null
            }
            Copy-Item $AltPath $DllPath -Force
            # Clean up the config subdirectory since we moved the DLL out of it
            $ConfigSubDir = Split-Path $AltPath -Parent
            if ((Get-ChildItem $ConfigSubDir | Measure-Object).Count -eq 0) {
                Remove-Item $ConfigSubDir -Force -ErrorAction SilentlyContinue
            }
            Write-Host "  Moved from $AltPath to Prebuilt\Win64\"
            break
        }
    }
}

if (Test-Path $DllPath) {
    $size = (Get-Item $DllPath).Length
    $sizeKB = [math]::Round($size / 1024, 1)
    Write-Host "`n==========================================================" -ForegroundColor Green
    Write-Host " BUILD SUCCEEDED" -ForegroundColor Green
    Write-Host "==========================================================" -ForegroundColor Green
    Write-Host "  Output: $DllPath"
    Write-Host "  Size:   $sizeKB KB"
    Write-Host "==========================================================" -ForegroundColor Green
} else {
    Write-Warning "Build completed but GorgeousVault.dll was not found at expected location."
    Write-Warning "Check the build output above for the actual output path."
    Write-Host "Expected: $DllPath"
    
    # List what was actually produced
    if (Test-Path $BuildDir) {
        Write-Host "`nFiles in build directory:"
        Get-ChildItem -Recurse $BuildDir -Filter "GorgeousVault*" | ForEach-Object {
            Write-Host "  $($_.FullName)"
        }
    }
}
