<#
.SYNOPSIS
  Local development helper: configure / build / run.

.DESCRIPTION
  ASCII only on purpose: Windows PowerShell 5.1 decodes BOM-less scripts
  with the ANSI code page, so non-ASCII text would break parsing.

.EXAMPLE
  .\scripts\windows\dev.ps1
  .\scripts\windows\dev.ps1 -Action configure
  .\scripts\windows\dev.ps1 -Action build -Target Bubble
  .\scripts\windows\dev.ps1 -Action test
  .\scripts\windows\dev.ps1 -Action run -Target Bubble
#>
[CmdletBinding()]
param(
  [ValidateSet('configure', 'build', 'run', 'test')]
  [string]$Action = 'build',
  [string]$Target = 'all',
  [string]$BuildType = 'Debug',
  [string]$BuildDir = 'build',
  [string]$QtDir = 'C:\Qt\6.12.0\msvc2022_64',
  [string[]]$AppArgs = @()
)

$ErrorActionPreference = 'Stop'

$ProjectDir = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$BuildPath = Join-Path $ProjectDir $BuildDir
$CMake = 'C:\Qt\Tools\CMake_64\bin\cmake.exe'
if (-not (Test-Path $CMake)) { $CMake = 'cmake' }

# PowerShell 5.1 reports native stderr as error records; keep them as plain text
# and rely on the exit code instead.
function Invoke-Native {
  param([string]$Exe, [string[]]$Arguments)
  $previous = $ErrorActionPreference
  $ErrorActionPreference = 'Continue'
  try {
    & $Exe @Arguments 2>&1 | ForEach-Object {
      if ($_ -is [System.Management.Automation.ErrorRecord]) { Write-Host $_.Exception.Message }
      else { Write-Host $_ }
    }
    return $LASTEXITCODE
  }
  finally {
    $ErrorActionPreference = $previous
  }
}

function Invoke-Configure {
  param([string[]]$ExtraArgs = @())
  $ninja = 'C:\Qt\Tools\Ninja\ninja.exe'
  if (Test-Path $ninja) { $env:CMAKE_MAKE_PROGRAM = $ninja }

  $cmakeArgs = @(
    '-S', $ProjectDir,
    '-B', $BuildPath,
    '-G', 'Ninja',
    "-DCMAKE_BUILD_TYPE=$BuildType",
    "-DQT_ROOT=$QtDir",
    '-DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE'
  ) + $ExtraArgs

  $code = Invoke-Native $CMake $cmakeArgs
  if ($code) { throw "cmake configure failed (exit $code)" }
}

function Invoke-Build {
  param([string]$TargetName = 'all')
  $code = Invoke-Native $CMake @(
    '--build', $BuildPath,
    '--config', $BuildType,
    '--target', $TargetName
  )
  if ($code) { throw "cmake build failed (exit $code)" }
}

if ($Action -eq 'run') {
  if ($Target -eq 'all') { throw "run needs a concrete example target (e.g. -Target Bubble)" }
  $exe = Join-Path $ProjectDir "binaries\cmake\x64\bin\$($BuildType.ToLower())\$Target.exe"
  if (-not (Test-Path $exe)) { throw "Not found: $exe (run -Action build -Target $Target first)" }
  $env:PATH = "$QtDir\bin;$env:PATH"
  & $exe @AppArgs
  exit $LASTEXITCODE
}

& (Join-Path $ProjectDir 'packaging\windows\Enter-VsDevShell.ps1') -Arch x64 -HostArch x64
if ($LASTEXITCODE) { throw "Failed to enter DevShell (exit $LASTEXITCODE)" }

# 让 cl 的诊断恒为英文：CMake/Ninja 用 "Note: including file:" 建立头文件
# 依赖，系统语言为中文而编译器输出英文时前缀对不上，会导致改头文件不重编。
$env:VSLANG = '1033'

if ($Action -eq 'configure') {
  Invoke-Configure
  exit 0
}

if ($Action -eq 'test') {
  # Tests are built by default (BUILD_TESTS=ON); configure and build first.
  Invoke-Configure
  Invoke-Build

  $ctest = Join-Path (Split-Path $CMake) 'ctest.exe'
  if (-not (Test-Path $ctest)) { $ctest = 'ctest' }
  # Test executables sit in binaries\...\bin\<config>\, away from the Qt
  # runtime DLLs, so add the Qt directory to PATH.
  $env:PATH = "$QtDir\bin;$env:PATH"

  $code = Invoke-Native $ctest @(
    '--test-dir', $BuildPath,
    '--output-on-failure',
    '--build-config', $BuildType
  )
  exit $code
}

if (-not (Test-Path (Join-Path $BuildPath 'CMakeCache.txt'))) {
  throw "Build directory is not configured, run -Action configure first"
}

Invoke-Build -TargetName $Target
