#Requires -Version 7.0
<#
.SYNOPSIS
  Builds the plugin, runs CMake install, then compiles the Inno Setup installer (.exe).

.EXAMPLE
  .\build-installer.ps1
  .\build-installer.ps1 -Configuration RelWithDebInfo -SkipDeps
#>
[CmdletBinding()]
param(
    [ValidateSet('Release', 'RelWithDebInfo', 'Debug')]
    [string] $Configuration = 'Release',
    [switch] $SkipDeps
)

$ErrorActionPreference = 'Stop'
$ProjectRoot = $PSScriptRoot
$Scripts = Join-Path $ProjectRoot '.github\scripts'

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path $vswhere)) {
    throw "vswhere.exe not found. Install Visual Studio 2022 or later with C++ workload."
}
$vsInstall = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath |
    Select-Object -First 1
if (-not $vsInstall) {
    throw "Visual Studio C++ tools not found. Install MSVC v143 or later."
}
$vcvars = Join-Path $vsInstall 'VC\Auxiliary\Build\vcvars64.bat'
if (-not (Test-Path $vcvars)) {
    throw "vcvars64.bat not found at $vcvars"
}

function Invoke-InVsDevEnvironment {
    param(
        [Parameter(Mandatory)]
        [string] $ArgumentString
    )
    $inner = "call `"$vcvars`" >nul 2>&1 && cd /d `"$ProjectRoot`" && $ArgumentString"
    cmd /c $inner
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed (exit $LASTEXITCODE): $ArgumentString"
    }
}

Push-Location $ProjectRoot
try {
    $buildArgs = "-NoProfile -File `"$Scripts\Build-Windows.ps1`" -Configuration $Configuration"
    if ($SkipDeps) { $buildArgs += ' -SkipDeps' }
    Invoke-InVsDevEnvironment "pwsh $buildArgs"

    $pkgArgs = "-NoProfile -File `"$Scripts\Package-Windows.ps1`" -Configuration $Configuration -BuildInstaller"
    if ($SkipDeps) { $pkgArgs += ' -SkipDeps' }
    Invoke-InVsDevEnvironment "pwsh $pkgArgs"

    $spec = Get-Content (Join-Path $ProjectRoot 'buildspec.json') -Raw | ConvertFrom-Json
    $exeName = "{0}-{1}-windows-x64-Installer.exe" -f $spec.name, $spec.version
    $exePath = Join-Path $ProjectRoot "release\$exeName"
    if (Test-Path $exePath) {
        Write-Host ""
        Write-Host "Installer ready: $exePath" -ForegroundColor Green
    } else {
        Write-Warning "Expected installer not found at $exePath (check iscc output above)."
    }
} finally {
    Pop-Location
}
