@echo off
setlocal
echo ========================================================
echo   Starting DHUN Music Playlist Server
echo ========================================================

REM Add local toolchain to PATH if available
if exist "%~dp0.tools\w64devkit\bin\g++.exe" (
    set "PATH=%~dp0.tools\w64devkit\bin;%PATH%"
)

if not exist "%~dp0build\dhun_server.exe" (
    echo dhun_server.exe not found. Building now...
    call "%~dp0build.bat"
)

echo Starting DHUN on http://localhost:8080 ...
"%~dp0build\dhun_server.exe"
