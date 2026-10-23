<#
.SYNOPSIS
  Enter the Visual Studio developer shell (DevShell) environment.

.DESCRIPTION
  This file is ASCII only on purpose: Windows PowerShell 5.1 decodes
  BOM-less scripts with the ANSI code page, so non-ASCII text would break
  parsing for anyone running this file on a non-UTF-8 code page.

.PARAMETER VersionRange
  VS version range to match, e.g. "[16.0,18.0)". Defaults to the latest release.

.PARAMETER Arch
  Target architecture: x86|x64|arm64. Defaults to x64.

.PARAMETER HostArch
  Host architecture: x86|x64|arm64. Defaults to x64.

.PARAMETER Preview
  Also match preview builds of VS. Defaults to release builds only.
#>
[CmdletBinding()]
param(
  [string]$VersionRange,
  [ValidateSet('x86', 'x64', 'arm64')]
  [string]$Arch = 'x64',
  [ValidateSet('x86', 'x64', 'arm64')]
  [string]$HostArch = 'x64',
  [switch]$Preview
)

# 1. Build the vswhere command line ------------------------------------------
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vsWhere)) {
  Write-Error "vswhere.exe not found, check that the Visual Studio Installer exists."
  exit 0x80070002
}

$argList = @(
  '-format', 'json'
  '-products', '*'
  '-requires', 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64'
)
if ($Preview) { $argList += '-prerelease' }
if ($VersionRange) { $argList += '-version', $VersionRange }
else { $argList += '-latest' }

# 2. Resolve the installation path -------------------------------------------
$installations = @(& $vsWhere $argList | ConvertFrom-Json)
if (-not $installations.Count) {
  Write-Warning "vswhere found no matching Visual Studio installation, trying well-known paths."
  foreach ($base in @("${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Enterprise",
      "${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise")) {
    if (Test-Path $base) { $installations = @([PSCustomObject]@{installationPath = $base }) }
  }
  if (-not $installations.Count) {
    Write-Error "No Visual Studio installation detected."
    exit 0x80070003
  }
}

$vsPath = $installations[0].installationPath
Write-Host "Visual Studio: $vsPath" -ForegroundColor Cyan

# 3. Load DevShell and enter the environment ---------------------------------
$devShellDll = Join-Path $vsPath "Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
if (-not (Test-Path $devShellDll)) {
  Write-Error "DevShell DLL not found: $devShellDll"
  exit 0x80070004
}

try {
  Import-Module $devShellDll -ErrorAction Stop
  Enter-VsDevShell -VsInstallPath $vsPath `
    -DevCmdArguments "-arch=$Arch -host_arch=$HostArch"
  Write-Host "DevShell ready (arch=$Arch, host_arch=$HostArch)." -ForegroundColor Green
}
catch {
  Write-Error "Failed to enter DevShell: $_"
  exit 0x80070005
}
