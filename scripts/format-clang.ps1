param(
    [string]$Root = "${PSScriptRoot}/.."
)

Write-Host "Running clang-format across repository..."

$exe = "clang-format"
if (-not (Get-Command $exe -ErrorAction SilentlyContinue)) {
    Write-Error "clang-format not found in PATH. Install LLVM/clang or add clang-format to PATH.";
    exit 2
}

$patterns = @('**/*.h','**/*.hpp','**/*.c','**/*.cpp','**/*.cc','**/*.cxx')

foreach ($p in $patterns) {
    Get-ChildItem -Path $Root -Include $p -Recurse -File -ErrorAction SilentlyContinue | ForEach-Object {
        Write-Host "Formatting $($_.FullName)"
        & $exe -i $_.FullName
    }
}

Write-Host "clang-format run complete."
