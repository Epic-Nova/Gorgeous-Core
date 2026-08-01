param(
    [Parameter(Mandatory = $true)]
    [string]$EngineRoot,
    [switch]$Apply
)

$VersionFile = Join-Path $EngineRoot "Engine/Build/Build.version"
if (-not (Test-Path -LiteralPath $VersionFile))
{
    throw "Build.version was not found below '$EngineRoot'."
}

$Version = Get-Content -Raw -LiteralPath $VersionFile | ConvertFrom-Json
$UseLegacyStructUtils = $Version.MajorVersion -lt 5 -or ($Version.MajorVersion -eq 5 -and $Version.MinorVersion -lt 6)
$PluginRoot = Split-Path -Parent $PSScriptRoot
$Descriptors = @(
    (Join-Path $PluginRoot "GorgeousCore.uplugin"),
    (Join-Path (Join-Path (Split-Path -Parent $PluginRoot) "Gorgeous-Events") "GorgeousEvents.uplugin")
)

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

    $Json.Plugins = $Plugins
    $Action = if ($UseLegacyStructUtils) { "require StructUtils" } else { "remove StructUtils" }
    Write-Host "$(Split-Path -Leaf $Descriptor): $Action"

    if ($Apply)
    {
        $Json | ConvertTo-Json -Depth 20 | Set-Content -LiteralPath $Descriptor -NoNewline
    }
}
