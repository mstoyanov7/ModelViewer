@echo off
setlocal

REM === Define build folder ===
set BUILD_DIR=build

REM === Remove build directory if it exists ===
if exist %BUILD_DIR% (
    rmdir /S /Q %BUILD_DIR%
    echo Build folder cleaned.
) else (
    echo Nothing to clean.
)

endlocal
pause