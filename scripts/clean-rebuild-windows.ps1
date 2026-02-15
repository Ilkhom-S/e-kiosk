# Clean and rebuild all targets on Windows (MinGW Qt6)
# This ensures plugins are rebuilt with latest interfaces

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
$BuildDir = Join-Path $ProjectRoot "build\win-mingw-x64"

Write-Host "===================================" -ForegroundColor Cyan
Write-Host "Clean Rebuild for Windows Qt6" -ForegroundColor Cyan
Write-Host "===================================" -ForegroundColor Cyan

# Remove build directory
if (Test-Path $BuildDir) {
    Write-Host "Removing existing build directory: $BuildDir" -ForegroundColor Yellow
    Remove-Item -Path $BuildDir -Recurse -Force
}

# Create fresh build directory
Write-Host "Creating fresh build directory..." -ForegroundColor Green
New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null

# Configure CMake
Write-Host "Configuring CMake..." -ForegroundColor Green
Push-Location $BuildDir
try {
    cmake -S $ProjectRoot `
          -B $BuildDir `
          -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/mingw_64" `
          -DCMAKE_BUILD_TYPE=Debug `
          -G "MinGW Makefiles"

    # Build all targets (includes plugins)
    Write-Host "Building all targets..." -ForegroundColor Green
    cmake --build $BuildDir -j8
} finally {
    Pop-Location
}

Write-Host ""
Write-Host "===================================" -ForegroundColor Cyan
Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "===================================" -ForegroundColor Cyan
Write-Host "Binaries location: $BuildDir\bin\" -ForegroundColor White
Write-Host "Plugins location: $BuildDir\bin\plugins\" -ForegroundColor White
