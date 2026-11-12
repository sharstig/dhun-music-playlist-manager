Write-Host "========================================================" -ForegroundColor Cyan
Write-Host "  Running DHUN DSA Unit Tests (Doubly Circular List)    " -ForegroundColor Cyan
Write-Host "========================================================" -ForegroundColor Cyan

$toolsDir = Join-Path $PSScriptRoot ".tools\w64devkit\bin"
if (Test-Path (Join-Path $toolsDir "g++.exe")) {
    $env:Path = "$toolsDir;" + $env:Path
}

$testExe = Join-Path $PSScriptRoot "build\dhun_tests.exe"
if (-not (Test-Path $testExe)) {
    Write-Host "dhun_tests.exe not found. Running build..." -ForegroundColor Yellow
    & "$PSScriptRoot\build.ps1"
}

& $testExe
