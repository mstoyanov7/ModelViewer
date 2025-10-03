@echo off
setlocal

REM === Define build folder ===
set BUILD_DIR=build

REM === Create build directory if it doesn't exist ===
if not exist %BUILD_DIR% (
    mkdir %BUILD_DIR%
)

REM === Go into build directory ===
cd %BUILD_DIR%

REM === Run CMake configure (default generator for MSVC) ===
cmake .. -G "Visual Studio 17 2022" -A x64

REM === Build in Release mode ===
cmake --build . --config Release

cd ..
endlocal
pause