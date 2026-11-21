param([Parameter(Mandatory = $true)][string]$Inventory)

$ErrorActionPreference = 'Stop'

# On a new target UBT validates the folders while it evaluates the module rules.
if (!(Test-Path -LiteralPath $Inventory -PathType Leaf)) { return }

function Resolve-GuardPath([string]$Root, [string]$RelativePath) {
    $path = [IO.Path]::GetFullPath([IO.Path]::Combine($Root, $RelativePath))
    if (!$path.StartsWith($Root + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Guard manifest path leaves its plugin: $RelativePath"
    }
    return $path
}

foreach ($manifest in (Get-Content -Raw -LiteralPath $Inventory | ConvertFrom-Json)) {
    # A removed manifest invalidates the makefile and is handled by the module rules.
    if (!(Test-Path -LiteralPath $manifest -PathType Leaf)) { continue }
    $document = Get-Content -Raw -LiteralPath $manifest | ConvertFrom-Json
    $root = Split-Path -Parent (Split-Path -Parent $manifest)
    $expected = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
    foreach ($source in $document.Sources) {
        [void]$expected.Add((Resolve-GuardPath $root $source.Path))
    }

    $actual = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
    foreach ($folder in $document.GuardFolders) {
        $path = Resolve-GuardPath $root $folder
        if (Test-Path -LiteralPath $path -PathType Container) {
            foreach ($file in [IO.Directory]::EnumerateFiles($path, '*.uasset', [IO.SearchOption]::AllDirectories)) {
                [void]$actual.Add($file)
            }
        }
    }
    if (!$expected.SetEquals($actual)) {
        throw "Added/removed guard source assets require a fresh export: $manifest"
    }
}
