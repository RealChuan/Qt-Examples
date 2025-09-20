<#
.SYNOPSIS
  一键进入 Visual Studio 开发者命令行环境（DevShell）。

.PARAMETER VersionRange
  要筛选的 VS 版本范围，例如 "[16.0,18.0)"；默认取最新正式版。

.PARAMETER Arch
  目标架构：x86|x64|arm64；默认 x64。

.PARAMETER HostArch
  Host 架构：x86|x64|arm64；默认 x64。

.PARAMETER Preview
  允许匹配 Preview 版 VS；默认只找正式版。
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

# 1. 拼 vswhere 命令 ----------------------------------------------------------
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vsWhere)) {
  Write-Error "vswhere.exe 未找到，请确认 VS Installer 存在。"
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

# 2. 拿安装路径 ---------------------------------------------------------------
$installations = @(& $vsWhere $argList | ConvertFrom-Json)
if (-not $installations.Count) {
  Write-Warning "vswhere 未找到任何符合条件的 VS 安装，尝试固定目录兜底。"
  foreach ($base in @("${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Enterprise",
      "${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise")) {
    if (Test-Path $base) { $installations = @([PSCustomObject]@{installationPath = $base }) }
  }
  if (-not $installations.Count) {
    Write-Error "未检测到任何 VS 安装。"
    exit 0x80070003
  }
}

$vsPath = $installations[0].installationPath
Write-Host "选中 VS 安装路径：$vsPath" -ForegroundColor Cyan

# 3. 加载 DevShell 并进入环境 -------------------------------------------------
$devShellDll = Join-Path $vsPath "Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
if (-not (Test-Path $devShellDll)) {
  Write-Error "DevShell DLL 不存在：$devShellDll"
  exit 0x80070004
}

try {
  Import-Module $devShellDll -ErrorAction Stop
  Enter-VsDevShell -VsInstallPath $vsPath `
    -DevCmdArguments "-arch=$Arch -host_arch=$HostArch"
  Write-Host "DevShell 环境已启动 (arch=$Arch, host_arch=$HostArch)。" -ForegroundColor Green
}
catch {
  Write-Error "进入 DevShell 失败：$_"
  exit 0x80070005
}
