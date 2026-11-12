@echo off
setlocal
echo ========================================================
echo   Running DHUN DSA Unit Tests (Doubly Circular List)
echo ========================================================

REM Add local toolchain to PATH if available
if exist "%~dp0.tools\w64devkit\bin\g++.exe" (
    set "PATH=%~dp0.tools\w64devkit\bin;%PATH%"
)

if not exist "%~dp0build\dhun_tests.exe" (
    echo dhun_tests.exe not found. Building now...
    call "%~dp0build.bat"
)

"%~dp0build\dhun_tests.exe"
