[CmdletBinding()]
param(
    [ValidateSet('x64')]
    [string] $Target = 'x64',
    [ValidateSet('Debug', 'RelWithDebInfo', 'Release', 'MinSizeRel')]
    [string] $Configuration = 'RelWithDebInfo',
    [switch] $BuildInstaller,
    [switch] $SkipDeps
)

$ErrorActionPreference = 'Stop'

if ( $DebugPreference -eq 'Continue' ) {
    $VerbosePreference = 'Continue'
    $InformationPreference = 'Continue'
}

if ( ! ( [System.Environment]::Is64BitOperatingSystem ) ) {
    throw "Packaging script requires a 64-bit system to build and run."
}


if ( $PSVersionTable.PSVersion -lt '7.0.0' ) {
    Write-Warning 'The packaging script requires PowerShell Core 7. Install or upgrade your PowerShell version: https://aka.ms/pscore6'
    exit 2
}

function Package {
    trap {
        Pop-Location -Stack BuildTemp -ErrorAction 'SilentlyContinue'
        Write-Error $_
        Log-Group
        exit 2
    }

    $ScriptHome = $PSScriptRoot
    $ProjectRoot = Resolve-Path -Path "$PSScriptRoot/../.."
    $BuildSpecFile = "${ProjectRoot}/buildspec.json"

    $UtilityFunctions = Get-ChildItem -Path $PSScriptRoot/utils.pwsh/*.ps1 -Recurse

    foreach( $Utility in $UtilityFunctions ) {
        Write-Debug "Loading $($Utility.FullName)"
        . $Utility.FullName
    }

    $BuildSpec = Get-Content -Path ${BuildSpecFile} -Raw | ConvertFrom-Json
    $ProductName = $BuildSpec.name
    $ProductVersion = $BuildSpec.version

    $OutputName = "${ProductName}-${ProductVersion}-windows-${Target}"

    if ( ! $SkipDeps ) {
        Install-BuildDependencies -WingetFile "${ScriptHome}/.Wingetfile"
    }

    $RemoveArgs = @{
        ErrorAction = 'SilentlyContinue'
        Path = @(
            "${ProjectRoot}/release/${ProductName}-*-windows-*.zip"
            "${ProjectRoot}/release/${ProductName}-*-windows-*.exe"
        )
    }

    Remove-Item @RemoveArgs

    Log-Group "Archiving ${ProductName}..."
    $CompressArgs = @{
        Path = (Get-ChildItem -Path "${ProjectRoot}/release/${Configuration}" -Exclude "${OutputName}*.*")
        CompressionLevel = 'Optimal'
        DestinationPath = "${ProjectRoot}/release/${OutputName}.zip"
        Verbose = ($Env:CI -ne $null)
    }
    Compress-Archive -Force @CompressArgs
    Log-Group

    if ( ( $BuildInstaller ) ) {
        Log-Group "Packaging ${ProductName}..."

        $IsccFile = "${ProjectRoot}/build_${Target}/installer-Windows.generated.iss"
        if ( ! ( Test-Path -Path $IsccFile ) ) {
            throw 'InnoSetup install script not found. Run the build script or the CMake build and install procedures first.'
        }

        $IsccExe = $null
        $InnoRoots = @(
            (Join-Path ([Environment]::GetFolderPath('ProgramFilesX86')) 'Inno Setup 6'),
            (Join-Path ([Environment]::GetFolderPath('ProgramFiles')) 'Inno Setup 6'),
            (Join-Path $env:LocalAppData 'Programs\Inno Setup 6')
        )
        foreach ( $Root in $InnoRoots ) {
            $Candidate = Join-Path $Root 'ISCC.exe'
            if ( Test-Path -LiteralPath $Candidate ) {
                $IsccExe = $Candidate
                break
            }
        }
        if ( -not $IsccExe ) {
            $cmd = Get-Command iscc -ErrorAction SilentlyContinue
            if ( $cmd ) { $IsccExe = $cmd.Source }
        }
        if ( -not $IsccExe ) {
            foreach ( $UninstallKey in @(
                    'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\*',
                    'HKLM:\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\*'
                ) ) {
                $Entries = Get-ItemProperty $UninstallKey -ErrorAction SilentlyContinue |
                    Where-Object { $_.DisplayName -like 'Inno Setup*' }
                foreach ( $Entry in $Entries ) {
                    if ( $Entry.InstallLocation ) {
                        $Candidate = Join-Path $Entry.InstallLocation.TrimEnd('\') 'ISCC.exe'
                        if ( Test-Path -LiteralPath $Candidate ) {
                            $IsccExe = $Candidate
                            break
                        }
                    }
                }
                if ( $IsccExe ) { break }
            }
        }
        if ( -not $IsccExe ) {
            throw 'Inno Setup 6 compiler (ISCC.exe) not found. Install Inno Setup 6 or add it to PATH: winget install JRSoftware.InnoSetup'
        }

        Log-Information "Creating InnoSetup installer using ${IsccExe}..."
        Push-Location -Stack BuildTemp
        try {
            $StageDir = Join-Path $ProjectRoot 'release/Package'
            $SourceDir = Join-Path $ProjectRoot "release/${Configuration}"
            Remove-Item -LiteralPath $StageDir -Recurse -Force -ErrorAction SilentlyContinue
            New-Item -ItemType Directory -Path $StageDir -Force | Out-Null
            Copy-Item -Path (Join-Path $SourceDir '*') -Destination $StageDir -Recurse -Force
            Invoke-External $IsccExe ${IsccFile} /O"${ProjectRoot}/release" /F"${OutputName}-Installer"
        } finally {
            Remove-Item -LiteralPath $StageDir -Recurse -Force -ErrorAction SilentlyContinue
            Pop-Location -Stack BuildTemp
        }

        Log-Group
    }
}

Package
