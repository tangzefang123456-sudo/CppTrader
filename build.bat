@echo off
setlocal

REM Create build log file
echo Build started at %date% %time% > compiler.log
echo. >> compiler.log

REM Build memory pool library
msbuild MemoryPool\MemoryPool.vcxproj /p:Configuration=Release /p:Platform=x64 >> compiler.log 2>&1
if %errorlevel% neq 0 (
    echo Memory pool library build failed >> compiler.log
    goto error
)

echo Memory pool library build succeeded >> compiler.log

REM Build test project
msbuild MemoryPoolTests\MemoryPoolTests.vcxproj /p:Configuration=Release /p:Platform=x64 >> compiler.log 2>&1
if %errorlevel% neq 0 (
    echo Test project build failed >> compiler.log
    goto error
)

echo Test project build succeeded >> compiler.log

REM Run tests
MemoryPoolTests\x64\Release\MemoryPoolTests.exe >> compiler.log 2>&1
if %errorlevel% neq 0 (
    echo Tests failed >> compiler.log
    goto error
)

echo Tests passed >> compiler.log
echo. >> compiler.log
echo Build completed at %date% %time% >> compiler.log
goto end

:error
echo Error occurred during build >> compiler.log
exit /b 1

:end
exit /b 0
