Write-Host "========================================================" -ForegroundColor Cyan
Write-Host "  Starting DHUN Music Playlist Server                   " -ForegroundColor Cyan
Write-Host "========================================================" -ForegroundColor Cyan

$toolsDir = Join-Path $PSScriptRoot ".tools\w64devkit\bin"
if (Test-Path (Join-Path $toolsDir "g++.exe")) {
    $env:Path = "$toolsDir;" + $env:Path
}

$serverExe = Join-Path $PSScriptRoot "build\dhun_server.exe"
if (-not (Test-Path $serverExe)) {
    Write-Host "dhun_server.exe not found. Running build..." -ForegroundColor Yellow
    & "$PSScriptRoot\build.ps1"
}

Write-Host "Starting DHUN on http://localhost:8080 ..." -ForegroundColor Green
Write-Host "Press Ctrl+C to stop the server.`n" -ForegroundColor DarkGray

& $serverExe
