<#
.SYNOPSIS
    Format the project sources with clang-format.

.DESCRIPTION
    Uses the clang-format bundled with Qt Creator by default. The style comes
    from the .clang-format file at the repository root.

    ASCII only on purpose: Windows PowerShell 5.1 decodes BOM-less scripts with
    the ANSI code page, so non-ASCII text would break parsing.

.PARAMETER Path
    Files or directories to format, relative to the repository root.
    Defaults to "src" and "tests".

.PARAMETER ClangFormat
    Path to clang-format.exe. Falls back to clang-format on PATH.

.PARAMETER Check
    Do not modify anything. Exit with an error when a file needs formatting.

.PARAMETER IncludeVendored
    Also format src/3rdparty, which is skipped by default.

.EXAMPLE
    .\scripts\windows\format.ps1

.EXAMPLE
    .\scripts\windows\format.ps1 -Check

.EXAMPLE
    .\scripts\windows\format.ps1 -Path src\plugins -IncludeVendored
#>
[CmdletBinding()]
param(
    [string[]]$Path = @('src', 'tests'),
    [string]$ClangFormat = 'C:\Qt\Tools\QtCreator\bin\clang\bin\clang-format.exe',
    [switch]$Check,
    [switch]$IncludeVendored
)

$ErrorActionPreference = 'Stop'

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$StyleFile = Join-Path $RepoRoot '.clang-format'

if (-not (Test-Path $ClangFormat)) {
    $OnPath = Get-Command clang-format -ErrorAction SilentlyContinue
    if (-not $OnPath) {
        throw "clang-format not found. Pass -ClangFormat <path> or install it on PATH."
    }
    $ClangFormat = $OnPath.Source
}

if (-not (Test-Path $StyleFile)) {
    throw "Missing style file: $StyleFile"
}

$Extensions = @('.h', '.hpp', '.cc', '.cpp')

$Files = @()
foreach ($Item in $Path) {
    $Resolved = Join-Path $RepoRoot $Item
    if (-not (Test-Path $Resolved)) {
        Write-Warning "Skipping missing path: $Item"
        continue
    }

    $Found = Get-ChildItem -Path $Resolved -Recurse -File |
    Where-Object { $Extensions -contains $_.Extension }
    $Files += $Found
}

$Files = $Files | Where-Object {
    $Relative = $_.FullName.Substring($RepoRoot.Length + 1)

    # Build output and packaging trees never contain hand written sources.
    if ($Relative -match '^(build|binaries|packaging)[\\/]') { return $false }

    # Vendored third party code keeps its own style unless asked otherwise.
    if (-not $IncludeVendored -and $Relative -match '^src[\\/]3rdparty[\\/]') { return $false }

    return $true
} | Sort-Object -Property FullName -Unique

if ($Files.Count -eq 0) {
    Write-Host 'No source files found.'
    exit 0
}

Write-Host "clang-format : $ClangFormat"
Write-Host "style        : $StyleFile"
Write-Host "files        : $($Files.Count)"
Write-Host ("vendored     : " + $(if ($IncludeVendored) { 'included' } else { 'excluded (-IncludeVendored to include)' }))

if ($Check) {
    $NeedsFormat = 0
    foreach ($File in $Files) {
        & $ClangFormat --style=file --assume-filename="$StyleFile" --dry-run --Werror $File.FullName
        if ($LASTEXITCODE -ne 0) {
            Write-Host "needs formatting: $($File.FullName)"
            $NeedsFormat++
        }
    }

    if ($NeedsFormat -gt 0) {
        Write-Host "$NeedsFormat file(s) need formatting. Run without -Check to rewrite them."
        exit 1
    }

    Write-Host 'All files are formatted.'
    exit 0
}

$Failed = 0
foreach ($File in $Files) {
    & $ClangFormat --style=file --assume-filename="$StyleFile" -i $File.FullName
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "Failed: $($File.FullName)"
        $Failed++
    }
}

if ($Failed -gt 0) {
    throw "$Failed file(s) could not be formatted."
}

Write-Host "Formatted $($Files.Count) file(s)."
