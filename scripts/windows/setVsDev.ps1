param(
    [Parameter(Mandatory = $false)]
    [string]$VersionRange,
    [Parameter(Mandatory = $false)]
    [string]$Arch
)


$vswhereArgs = @()

if ($VersionRange) {
    $vswhereArgs += "-version"
    $vswhereArgs += $VersionRange
}
else {
    $vswhereArgs += "-latest"
}


$vswhereArgs += "-property"
$vswhereArgs += "installationPath"

if ([string]::IsNullOrEmpty($Arch)) {
    $Arch = "x64"  
}

$Host_Arch = $Arch
if ($Arch -eq "arm64") {
    $Host_Arch = "x64"
}

Write-Host "Architecture: $Arch"
Write-Host "VSWhere Args: $vswhereArgs"

# 使用 vswhere 获取 Visual Studio 的安装路径
$vswherePath = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
$vsInstallPath = & $vswherePath $vswhereArgs | ForEach-Object { $_ }
# Output the results
Write-Host "Visual Studio installation paths:"
$vsInstallPath 

if ($null -ne $vsInstallPath) {
    $vsDevShell = Join-Path $vsInstallPath "Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
    if (-not (Test-Path $vsDevShell)) {
        Write-Host "Failed to find Visual Studio DevShell DLL: $vsDevShell"
        exit 1
    }
    Import-Module $vsDevShell
    Enter-VsDevShell -VsInstallPath $vsInstallPath -DevCmdArguments "-arch=$Arch -host_arch=$Host_Arch" -SkipAutomaticLocation

    if ($LASTEXITCODE -eq 0) {
        Write-Host "Development environment set up successfully."
    }
    else {
        Write-Host "Failed to set up the development environment."
    }
} 
else {
    Write-Host "Using Custom Visual Studio installation path."
    $vsInstallPath = "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise"
    if (-not (Test-Path $vsInstallPath)) {
        $vsInstallPath = "C:\Program Files\Microsoft Visual Studio\2022\Enterprise"
    }
    $vsDevShell = Join-Path $vsInstallPath "Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
    Import-Module $vsDevShell
    Enter-VsDevShell -VsInstallPath $vsInstallPath -DevCmdArguments "-arch=$Arch -host_arch=$Host_Arch" -SkipAutomaticLocation
}