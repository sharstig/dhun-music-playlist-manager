@echo off
setlocal
echo ========================================================
echo   Building DHUN - Music Playlist Management System
echo ========================================================

REM Add local toolchain to PATH if available
if exist "%~dp0.tools\w64devkit\bin\g++.exe" (
    set "PATH=%~dp0.tools\w64devkit\bin;%PATH%"
)

if not exist "%~dp0build" mkdir "%~dp0build"

echo Compiling C++17 Core Library and Server...
g++ -std=c++17 -O2 -Wall -Wextra -I"%~dp0include" "%~dp0src\DoublyCircularList.cpp" "%~dp0src\JsonHelper.cpp" "%~dp0src\HttpServer.cpp" "%~dp0src\main.cpp" -o "%~dp0build\dhun_server.exe" -lws2_32
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Server compilation failed!
    exit /b %ERRORLEVEL%
)

echo Compiling DSA Unit Tests...
g++ -std=c++17 -O2 -Wall -Wextra -I"%~dp0include" "%~dp0src\DoublyCircularList.cpp" "%~dp0tests\test_playlist.cpp" -o "%~dp0build\dhun_tests.exe"
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Unit test compilation failed!
    exit /b %ERRORLEVEL%
)

echo.
echo [SUCCESS] Build completed successfully!
echo Executables created in: %~dp0build\
echo   - dhun_server.exe
echo   - dhun_tests.exe
echo ========================================================
