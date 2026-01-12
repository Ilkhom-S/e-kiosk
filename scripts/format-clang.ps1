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
Get-ChildItem -Path (Join-Path -Path $Root -ChildPath '*') -Recurse -File -ErrorAction SilentlyContinue |
    Where-Object { $exts -contains $_.Extension.ToLower() } | ForEach-Object {
        Write-Host "Formatting $($_.FullName)"
        & $exe -i $_.FullName
    }

Write-Host "clang-format run complete."
