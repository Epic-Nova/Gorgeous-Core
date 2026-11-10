param(
    [Parameter(Mandatory = $true)]
    [string]$EngineRoot,
    [switch]$Apply,
    [string]$ProjectFile = ""
)

$VersionFile = Join-Path $EngineRoot "Engine/Build/Build.version"
if (-not (Test-Path -LiteralPath $VersionFile))
{
    throw "Build.version was not found below '$EngineRoot'."
}

$Version = Get-Content -Raw -LiteralPath $VersionFile | ConvertFrom-Json
$UseLegacyStructUtils = $Version.MajorVersion -lt 5 -or ($Version.MajorVersion -eq 5 -and $Version.MinorVersion -lt 6)
$PluginRoot = Split-Path -Parent $PSScriptRoot
$Descriptors = @((Join-Path $PluginRoot "GorgeousCore.uplugin"))
if ($ProjectFile) {
    $ProjectFile = (Resolve-Path -LiteralPath $ProjectFile -ErrorAction Stop).Path
} else {
    for ($Directory = Get-Item -LiteralPath $PluginRoot; $Directory; $Directory = $Directory.Parent) {
        $Projects = @(Get-ChildItem -LiteralPath $Directory.FullName -Filter *.uproject -File)
        if ($Projects.Count -gt 1) { throw "Several projects found below '$($Directory.FullName)'; pass -ProjectFile explicitly." }
        if ($Projects.Count -eq 1) { $ProjectFile = $Projects[0].FullName; break }
    }
}

# Match descriptor identity, not the folder's marketing/display name. Project
# roots override engine copies; duplicate matches within one scope are ambiguous.
$ProjectRoots = @()
if ($ProjectFile) {
    $ProjectDir = Split-Path -Parent $ProjectFile
    $ProjectRoots += Join-Path $ProjectDir 'Plugins'
    $Project = Get-Content -Raw -LiteralPath $ProjectFile | ConvertFrom-Json
    foreach ($Additional in @($Project.AdditionalPluginDirectories)) {
        if ($Additional) { $ProjectRoots += [IO.Path]::GetFullPath([IO.Path]::Combine($ProjectDir,$Additional)) }
    }
}
$SearchScopes = @(
    @{ Roots = $ProjectRoots },
    @{ Roots = @((Join-Path $EngineRoot 'Engine/Plugins')) },
    @{ Roots = @((Split-Path -Parent $PluginRoot)) }
)
foreach ($Scope in $SearchScopes) {
    $Candidates = @($Scope.Roots | Where-Object { Test-Path -LiteralPath $_ -PathType Container } |
        ForEach-Object { Get-ChildItem -LiteralPath $_ -Filter GorgeousEvents.uplugin -File -Recurse } |
        Select-Object -ExpandProperty FullName -Unique)
    if ($Candidates.Count -gt 1) { throw "Several GorgeousEvents descriptors found; use an unambiguous project plugin layout: $($Candidates -join ', ')" }
    if ($Candidates.Count -eq 1) { $Descriptors += $Candidates[0]; break }
}

foreach ($Descriptor in $Descriptors)
{
    if (-not (Test-Path -LiteralPath $Descriptor)) { continue }

    $Json = Get-Content -Raw -LiteralPath $Descriptor | ConvertFrom-Json -AsHashtable
    $Plugins = New-Object System.Collections.ArrayList
    foreach ($Entry in @($Json.Plugins))
    {
        if ($Entry.Name -ne "StructUtils")
        {
            [void]$Plugins.Add($Entry)
        }
    }

    if ($UseLegacyStructUtils)
    {
        [void]$Plugins.Add([ordered]@{ Name = "StructUtils"; Enabled = $true })
    }

    $Existing = @($Json.Plugins | Where-Object { $_.Name -eq "StructUtils" })
    if (($UseLegacyStructUtils -and $Existing.Count -eq 1 -and $Existing[0].Enabled) -or
        (-not $UseLegacyStructUtils -and $Existing.Count -eq 0)) {
        Write-Host "$(Split-Path -Leaf $Descriptor): already correct; unchanged"
        continue
    }
    $Json.Plugins = $Plugins
    $Action = if ($UseLegacyStructUtils) { "require StructUtils" } else { "remove StructUtils" }
    Write-Host "$(Split-Path -Leaf $Descriptor): $Action"

    if ($Apply)
    {
        $Json | ConvertTo-Json -Depth 20 | Set-Content -LiteralPath $Descriptor -NoNewline
    }
}
