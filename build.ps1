Write-Host "========================================================" -ForegroundColor Cyan
Write-Host "  Building DHUN - Music Playlist Management System      " -ForegroundColor Cyan
Write-Host "========================================================" -ForegroundColor Cyan

$toolsDir = Join-Path $PSScriptRoot ".tools\w64devkit\bin"
if (Test-Path (Join-Path $toolsDir "g++.exe")) {
    $env:Path = "$toolsDir;" + $env:Path
}

$buildDir = Join-Path $PSScriptRoot "build"
if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

Write-Host "Compiling C++17 Core Library and Server..." -ForegroundColor Yellow
& g++ -std=c++17 -O2 -Wall -Wextra -I"$PSScriptRoot\include" "$PSScriptRoot\src\DoublyCircularList.cpp" "$PSScriptRoot\src\JsonHelper.cpp" "$PSScriptRoot\src\HttpServer.cpp" "$PSScriptRoot\src\main.cpp" -o "$buildDir\dhun_server.exe" -lws2_32
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Server compilation failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "Compiling DSA Unit Tests..." -ForegroundColor Yellow
& g++ -std=c++17 -O2 -Wall -Wextra -I"$PSScriptRoot\include" "$PSScriptRoot\src\DoublyCircularList.cpp" "$PSScriptRoot\tests\test_playlist.cpp" -o "$buildDir\dhun_tests.exe"
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Unit test compilation failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "`n[SUCCESS] Build completed successfully!" -ForegroundColor Green
Write-Host "Executables located in: $buildDir" -ForegroundColor Green
Write-Host "  - dhun_server.exe"
Write-Host "  - dhun_tests.exe"
Write-Host "========================================================" -ForegroundColor Cyan
