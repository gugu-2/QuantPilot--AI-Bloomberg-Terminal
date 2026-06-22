<#
.SYNOPSIS
    Builds QuantPilot Terminal and packages it into a Windows Installer (.exe).

.DESCRIPTION
    Complete automated release build and packaging for QuantPilot Terminal.
    Steps:
    1. Validates prerequisites (CMake, Qt6, Qt Installer Framework).
    2. Configures & builds the Qt C++ project in Release mode via CMake.
    3. Runs windeployqt to collect all required Qt DLLs and plugins.
    4. Copies the Python scripts/agents to the installer data folder.
    5. Copies the app icon into the installer config folder.
    6. Runs binarycreator to produce QuantPilot_Setup_v1.0.exe.

.PREREQUISITES
    - Visual Studio 2022 (MSVC 19.40+)  OR  MinGW 12.3+
    - CMake 3.27+ on PATH
    - Qt 6.x installed via Qt Online Installer (default: C:\Qt\)
    - Qt Installer Framework installed via Qt Maintenance Tool
    - Windows 10/11

.USAGE
    # Default (auto-detects newest Qt 6 in C:\Qt):
    .\build_windows_installer.ps1

    # Specify Qt path explicitly:
    .\build_windows_installer.ps1 -QT_DIR "C:\Qt\6.8.1\msvc2022_64" -QT_IFW_DIR "C:\Qt\Tools\QtInstallerFramework\4.8\bin"
#>

param (
    [string]$QT_DIR     = "",   # Leave empty to auto-detect from C:\Qt
    [string]$QT_IFW_DIR = ""    # Leave empty to auto-detect from C:\Qt\Tools
)

$ErrorActionPreference = "Stop"
$ProgressPreference    = "SilentlyContinue"   # Faster Invoke-WebRequest

# --- Paths --------------------------------------------------------------------
$RepoRoot        = $PSScriptRoot
$SourceDir       = Join-Path $RepoRoot "fincept-qt"
$BuildDir        = Join-Path $SourceDir "build-release"
$PackagingDir    = Join-Path $SourceDir "packaging\installer"
$CoreDataDir     = Join-Path $PackagingDir "packages\com.quantpilot.terminal.core\data"
$ScriptsDataDir  = Join-Path $PackagingDir "packages\com.quantpilot.terminal.scripts\data"
$OutputInstaller = Join-Path $RepoRoot   "QuantPilot_Setup_v1.0.exe"
$IconSrc         = Join-Path $RepoRoot   "fincept_icon.ico"
$IconDest        = Join-Path $PackagingDir "config\quantpilot_install.ico"

Write-Host ""
Write-Host "========================================================" -ForegroundColor Cyan
Write-Host "  QuantPilot Terminal  -  Windows Installer Builder      " -ForegroundColor Cyan
Write-Host "========================================================" -ForegroundColor Cyan
Write-Host ""

# --- Auto-detect Qt -----------------------------------------------------------
function Find-QtDir {
    if (-not (Test-Path "C:\Qt")) { return $null }
    # Enumerate all msvc2022_64 sub-dirs, pick the newest Qt 6 version
    $dirs = Get-ChildItem "C:\Qt" -Directory |
        Where-Object { $_.Name -match "^6\." } |
        Sort-Object Name -Descending
    foreach ($d in $dirs) {
        $candidate = Join-Path $d.FullName "msvc2022_64"
        if (Test-Path (Join-Path $candidate "bin\qmake.exe")) {
            return $candidate
        }
        # Try mingw variant
        $mingwCandidates = Get-ChildItem $d.FullName -Directory | Where-Object { $_.Name -like "mingw*" }
        foreach ($m in $mingwCandidates) {
            if (Test-Path (Join-Path $m.FullName "bin\qmake.exe")) {
                return $m.FullName
            }
        }
    }
    return $null
}

function Find-IFWDir {
    if (-not (Test-Path "C:\Qt\Tools")) { return $null }
    $dirs = Get-ChildItem "C:\Qt\Tools" -Directory |
        Where-Object { $_.Name -like "QtInstallerFramework*" } |
        Sort-Object Name -Descending
    foreach ($d in $dirs) {
        $candidate = Join-Path $d.FullName "bin"
        if (Test-Path (Join-Path $candidate "binarycreator.exe")) {
            return $candidate
        }
    }
    return $null
}

if ([string]::IsNullOrWhiteSpace($QT_DIR)) {
    $QT_DIR = Find-QtDir
    if ($null -eq $QT_DIR) {
        Write-Error "Qt 6 not found in C:\Qt."
        Write-Error "Please install Qt 6 from https://www.qt.io/download-open-source"
        Write-Error "or pass the path explicitly:"
        Write-Error "  .\build_windows_installer.ps1 -QT_DIR 'C:\Qt\6.8.0\msvc2022_64'"
        exit 1
    }
    Write-Host "  Auto-detected Qt: $QT_DIR" -ForegroundColor Green
} else {
    if (-not (Test-Path "$QT_DIR\bin\qmake.exe")) {
        Write-Error "Qt not found at $QT_DIR (qmake.exe missing)."
        exit 1
    }
}

if ([string]::IsNullOrWhiteSpace($QT_IFW_DIR)) {
    $QT_IFW_DIR = Find-IFWDir
    if ($null -eq $QT_IFW_DIR) {
        Write-Error "Qt Installer Framework not found in C:\Qt\Tools."
        Write-Error "Please install it via the Qt Maintenance Tool (Developer and Designer Tools)."
        Write-Error "or pass the path explicitly:"
        Write-Error "  .\build_windows_installer.ps1 -QT_IFW_DIR 'C:\Qt\Tools\QtInstallerFramework\4.8\bin'"
        exit 1
    }
    Write-Host "  Auto-detected IFW: $QT_IFW_DIR" -ForegroundColor Green
} else {
    if (-not (Test-Path "$QT_IFW_DIR\binarycreator.exe")) {
        Write-Error "Qt Installer Framework not found at $QT_IFW_DIR (binarycreator.exe missing)."
        exit 1
    }
}

# --- Add Qt and IFW to PATH first ---------------------------------------------
$env:PATH = "$QT_DIR\bin;$QT_IFW_DIR;" + $env:PATH
$env:CMAKE_PREFIX_PATH = $QT_DIR

Write-Host ""
Write-Host "[1/5] Building QuantPilot (C++ Release)..." -ForegroundColor Yellow

# Clean and recreate build directory
if (Test-Path $BuildDir) { Remove-Item -Recurse -Force $BuildDir }
New-Item -ItemType Directory -Path $BuildDir | Out-Null
Set-Location $BuildDir

# CMake Configure - try Ninja first (faster), fall back to VS generator
$cmakeConfigured = $false
try {
    cmake -G "Ninja" `
          -DCMAKE_BUILD_TYPE=Release `
          "-DCMAKE_PREFIX_PATH=$QT_DIR" `
          "$SourceDir"
    if ($LASTEXITCODE -eq 0) { $cmakeConfigured = $true }
} catch {}

if (-not $cmakeConfigured) {
    Write-Host "  Ninja not available, falling back to VS generator..." -ForegroundColor DarkGray
    cmake -DCMAKE_BUILD_TYPE=Release `
          "-DCMAKE_PREFIX_PATH=$QT_DIR" `
          "$SourceDir"
    if ($LASTEXITCODE -ne 0) { Write-Error "CMake configure failed."; exit 1 }
}

# Build
cmake --build . --config Release --parallel
if ($LASTEXITCODE -ne 0) { Write-Error "CMake build failed."; exit 1 }
Write-Host "  Build successful." -ForegroundColor Green

# --- Locate built EXE ---------------------------------------------------------
$BuiltExe = Get-ChildItem -Path $BuildDir -Recurse -Filter "QuantPilot*.exe" |
    Where-Object { $_.DirectoryName -notlike "*_deps*" } |
    Select-Object -First 1
if ($null -eq $BuiltExe) {
    Write-Error "Could not find QuantPilot*.exe in build output. Check the CMakeLists.txt target name."
    exit 1
}
Write-Host "  Exe: $($BuiltExe.FullName)" -ForegroundColor DarkGray

Write-Host ""
Write-Host "[2/5] Deploying Qt Runtime Libraries..." -ForegroundColor Yellow

# Prepare core data directory
if (Test-Path $CoreDataDir) { Remove-Item -Recurse -Force "$CoreDataDir\*" }
else                         { New-Item -ItemType Directory -Path $CoreDataDir | Out-Null }

# Copy EXE
Copy-Item $BuiltExe.FullName -Destination $CoreDataDir

# windeployqt - collects all Qt DLLs, plugins, QML modules
$DeployedExe = Join-Path $CoreDataDir $BuiltExe.Name
windeployqt `
    --release `
    --no-translations `
    --compiler-runtime `
    "$DeployedExe"
if ($LASTEXITCODE -ne 0) { Write-Error "windeployqt failed."; exit 1 }

# Copy OpenSSL DLLs if present beside qmake (Qt ships them for Windows)
foreach ($dll in @("libssl-3-x64.dll","libcrypto-3-x64.dll","openssl*.dll")) {
    $found = Get-ChildItem "$QT_DIR\bin" -Filter $dll -ErrorAction SilentlyContinue
    foreach ($f in $found) { Copy-Item $f.FullName $CoreDataDir -ErrorAction SilentlyContinue }
}

Write-Host "  Qt runtime deployed." -ForegroundColor Green

Write-Host ""
Write-Host "[3/5] Packaging Python Scripts & Agents..." -ForegroundColor Yellow

# Prepare scripts data directory
if (Test-Path $ScriptsDataDir) { Remove-Item -Recurse -Force "$ScriptsDataDir\*" }
else                            { New-Item -ItemType Directory -Path $ScriptsDataDir | Out-Null }

$ScriptsSrc = Join-Path $SourceDir "scripts"
Copy-Item "$ScriptsSrc\*" -Destination $ScriptsDataDir -Recurse -Force
Write-Host "  Python scripts packaged." -ForegroundColor Green

Write-Host ""
Write-Host "[4/5] Copying App Icon..." -ForegroundColor Yellow
if (Test-Path $IconSrc) {
    Copy-Item $IconSrc -Destination $IconDest -Force
    Write-Host "  Icon copied." -ForegroundColor Green
} else {
    Write-Host "  Warning: fincept_icon.ico not found at $IconSrc - installer will use default icon." -ForegroundColor DarkYellow
}

Write-Host ""
Write-Host "[5/5] Building Windows Installer (.exe)..." -ForegroundColor Yellow
Set-Location $PackagingDir

binarycreator.exe `
    --offline-only `
    -c "config\config.xml" `
    -p packages `
    "$OutputInstaller"
if ($LASTEXITCODE -ne 0) { Write-Error "binarycreator failed."; exit 1 }

Set-Location $RepoRoot

$size = [math]::Round((Get-Item $OutputInstaller).Length / 1MB, 1)
Write-Host ""
Write-Host "========================================================" -ForegroundColor Green
Write-Host "  SUCCESS!" -ForegroundColor Green
Write-Host ""
Write-Host "  Installer : $OutputInstaller" -ForegroundColor White
Write-Host "  Size      : ${size} MB" -ForegroundColor White
Write-Host ""
Write-Host "  Distribute this single .exe file to your users." -ForegroundColor DarkGray
Write-Host "  It is self-contained - no Qt or Python needed on target machine." -ForegroundColor DarkGray
Write-Host "========================================================" -ForegroundColor Green
