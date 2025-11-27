@echo off
setlocal enabledelayedexpansion

REM Set Visual Studio installation path (modify if needed)
set VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community

REM Set build configuration and platform
set CONFIGURATION=Release
set PLATFORM=x64

REM Create build log file
echo Build started at %date% %time% > compiler.log
echo. >> compiler.log

REM Check if Visual Studio is installed
if not exist "%VS_PATH%" (
    echo Visual Studio 2022 not found at %VS_PATH% >> compiler.log
    echo Please install Visual Studio 2022 or modify the VS_PATH variable in this script. >> compiler.log
    echo Build failed >> compiler.log
    exit /b 1
)

REM Start Visual Studio Developer Command Prompt
call "%VS_PATH%\Common7\Tools\VsDevCmd.bat" -arch=%PLATFORM% -host_arch=%PLATFORM%

REM Build MemoryPool library
echo Building MemoryPool library... >> compiler.log
echo. >> compiler.log
msbuild MemoryPool\MemoryPool.vcxproj /p:Configuration=%CONFIGURATION% /p:Platform=%PLATFORM% >> compiler.log 2>&1
if !errorlevel! neq 0 (
    echo Memory pool library build failed >> compiler.log
    echo Error occurred during build >> compiler.log
    exit /b 1
)

echo MemoryPool library built successfully >> compiler.log
echo. >> compiler.log

REM Build MemoryPoolTests
echo Building MemoryPoolTests... >> compiler.log
echo. >> compiler.log
msbuild MemoryPoolTests\MemoryPoolTests.vcxproj /p:Configuration=%CONFIGURATION% /p:Platform=%PLATFORM% >> compiler.log 2>&1
if !errorlevel! neq 0 (
    echo Memory pool tests build failed >> compiler.log
    echo Error occurred during build >> compiler.log
    exit /b 1
)

echo MemoryPoolTests built successfully >> compiler.log
echo. >> compiler.log

REM Run tests
echo Running tests... >> compiler.log
echo. >> compiler.log
MemoryPoolTests\%CONFIGURATION%\MemoryPoolTests.exe >> compiler.log 2>&1
if !errorlevel! neq 0 (
    echo Tests failed >> compiler.log
    exit /b 1
)

echo Tests passed successfully >> compiler.log
echo. >> compiler.log
echo Build completed successfully at %date% %time% >> compiler.log

exit /b 0