[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

$outputDirectory = Join-Path $PSScriptRoot 'bin'
$winscardDirectory = Join-Path (Split-Path -Parent $PSScriptRoot) `
    'teknoparrot-winscard-stub'
$vswhere = Join-Path ${env:ProgramFiles(x86)} `
    'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path -LiteralPath $vswhere)) {
    throw 'Visual Studio Installer (vswhere.exe) was not found.'
}

$installation = & $vswhere -latest -products '*' `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath
if (-not $installation) {
    throw 'Install the Visual Studio C++ x86/x64 build tools first.'
}

$vsDevCmd = Join-Path $installation 'Common7\Tools\VsDevCmd.bat'
New-Item -ItemType Directory -Path $outputDirectory -Force | Out-Null

function Get-Sha256 {
    param([Parameter(Mandatory)] [string] $Path)

    $sha256 = [Security.Cryptography.SHA256]::Create()
    $stream = [IO.File]::OpenRead($Path)
    try {
        return ([BitConverter]::ToString($sha256.ComputeHash($stream)) -replace '-', '')
    }
    finally {
        $stream.Dispose()
        $sha256.Dispose()
    }
}

function Invoke-Compiler {
    param(
        [Parameter(Mandatory)] [ValidateSet('x64', 'x86')]
        [string] $Architecture,
        [Parameter(Mandatory)] [string] $Source,
        [Parameter(Mandatory)] [string] $Output,
        [string] $Libraries = '',
        [string] $ExtraCompile = '',
        [string] $ExtraLink = ''
    )

    $object = [IO.Path]::ChangeExtension($Output, ".${Architecture}.obj")
    $command = (
        'call "{0}" -no_logo -arch={1} -host_arch=x64 && ' +
        'cl.exe /nologo /W4 /WX /O2 /MT /D_CRT_SECURE_NO_WARNINGS ' +
        '{5} /Fo:"{2}" "{3}" /link {4} {6} /subsystem:console /out:"{7}"'
    ) -f $vsDevCmd, $Architecture, $object, $Source, $Libraries,
        $ExtraCompile, $ExtraLink, $Output
    & $env:ComSpec /d /c $command
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $Output)) {
        throw "Could not build $Output."
    }
}

Invoke-Compiler -Architecture x64 `
    -Source (Join-Path $PSScriptRoot 'winlator_bridge_guest.c') `
    -Output (Join-Path $outputDirectory 'bridgeguest64.exe')
Invoke-Compiler -Architecture x86 `
    -Source (Join-Path $PSScriptRoot 'winlator_bridge_guest.c') `
    -Output (Join-Path $outputDirectory 'bridgeguest32.exe')
Invoke-Compiler -Architecture x64 `
    -Source (Join-Path $PSScriptRoot 'pipehelper.c') `
    -Output (Join-Path $outputDirectory 'pipehelper.exe') `
    -Libraries 'ws2_32.lib'
Invoke-Compiler -Architecture x86 `
    -Source (Join-Path $PSScriptRoot 'pipehelper.c') `
    -Output (Join-Path $outputDirectory 'pipehelper32.exe') `
    -Libraries 'ws2_32.lib'
Invoke-Compiler -Architecture x86 `
    -Source (Join-Path $PSScriptRoot 'windows_path_bootstrap.c') `
    -Output (Join-Path $outputDirectory 'windows-path-bootstrap.exe') `
    -Libraries 'user32.lib'

$winscardOutput = Join-Path $outputDirectory 'winscard-x86.dll'
$winscardObject = Join-Path $outputDirectory 'winscard-x86.obj'
$winscardImportLibrary = Join-Path $outputDirectory 'winscard-x86.lib'
$winscardCommand = (
    'call "{0}" -no_logo -arch=x86 -host_arch=x64 && ' +
    'cl.exe /nologo /W4 /WX /O2 /MT /LD /D_CRT_SECURE_NO_WARNINGS ' +
    '/Fo:"{1}" "{2}" /link /DEF:"{3}" /IMPLIB:"{4}" /OUT:"{5}"'
) -f $vsDevCmd, $winscardObject,
    (Join-Path $winscardDirectory 'winscard_stub.c'),
    (Join-Path $winscardDirectory 'winscard_stub.def'),
    $winscardImportLibrary,
    $winscardOutput
& $env:ComSpec /d /c $winscardCommand
if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $winscardOutput)) {
    throw 'Could not build the x86 WinSCard compatibility stub.'
}

Copy-Item -LiteralPath (
    Join-Path $PSScriptRoot 'android-winlator-service-bridge-diagnostics.bat'
) -Destination $outputDirectory -Force

& (Join-Path $outputDirectory 'windows-path-bootstrap.exe') `
    --self-test-borderless
if ($LASTEXITCODE -ne 0) {
    throw 'The Windows borderless-window self-test failed.'
}

Get-ChildItem -LiteralPath $outputDirectory -File |
    Where-Object {
        $_.Extension -in '.exe', '.dll', '.bat'
    } |
    Sort-Object Name |
    ForEach-Object {
        $hash = Get-Sha256 -Path $_.FullName
        Write-Host "$($_.Name) $($_.Length) $hash"
    }
