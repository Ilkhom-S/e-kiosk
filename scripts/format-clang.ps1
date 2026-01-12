param(
    [string]$Root = "${PSScriptRoot}/.."
)

Write-Host "Running clang-format across repository..."

$exe = "clang-format"
if (-not (Get-Command $exe -ErrorAction SilentlyContinue)) {
    Write-Error "clang-format not found in PATH. Install LLVM/clang or add clang-format to PATH.";
    exit 2
}

# Use a robust extension list instead of relying on -Include with root path
$exts = @('.h', '.hpp', '.c', '.cpp', '.cc', '.cxx')
# Directories to exclude from formatting (build outputs, generated files, vendored code)
$excludeDirs = @('build', 'bin', 'out', 'thirdparty', 'node_modules', '.git')
# Build a regex that matches any of the exclude directory names in a path
$excludePattern = ($excludeDirs | ForEach-Object { [regex]::Escape($_) }) -join '|'

Get-ChildItem -Path (Join-Path -Path $Root -ChildPath '*') -Recurse -File -ErrorAction SilentlyContinue |
    Where-Object {
        $exts -contains $_.Extension.ToLower() -and -not ($_.FullName.ToLower() -match $excludePattern)
    } | ForEach-Object {
        Write-Host "Formatting $($_.FullName)"
        & $exe -i $_.FullName
    }

Write-Host "clang-format run complete."
