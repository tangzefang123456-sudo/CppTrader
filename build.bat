@echo off
setlocal enabledelayedexpansion

REM 设置项目根目录
set PROJECT_ROOT=%~dp0
set MODULES_DIR=%PROJECT_ROOT%modules
set CPPCOMMON_DIR=%MODULES_DIR%\cppcommon
set CATCH2_DIR=%MODULES_DIR%\Catch2

REM 检查是否安装了Git
git --version >nul 2>&1
if %errorlevel% neq 0 (
    echo 错误：未找到Git，请先安装Git
    echo 下载地址：https://git-scm.com/
    pause
    exit /b 1
)

REM 检查是否安装了Visual Studio 2022
if not exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
    if not exist "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" (
        if not exist "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe" (
            echo 错误：未找到Visual Studio 2022，请先安装
            echo 下载地址：https://visualstudio.microsoft.com/zh-hans/vs/
            pause
            exit /b 1
        ) else (
            set MSBUILD_PATH="%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
        )
    ) else (
        set MSBUILD_PATH="%ProgramFiles%\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
    )
) else (
    set MSBUILD_PATH="%ProgramFiles%\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
)

REM 克隆CppCommon库
if not exist "%CPPCOMMON_DIR%" (
    echo 克隆CppCommon库...
    git clone https://github.com/chronoxor/CppCommon.git "%CPPCOMMON_DIR%"
    if %errorlevel% neq 0 (
        echo 错误：克隆CppCommon库失败
        pause
        exit /b 1
    )
) else (
    echo CppCommon库已存在
)

REM 克隆Catch2库
if not exist "%CATCH2_DIR%" (
    echo 克隆Catch2库...
    git clone https://github.com/catchorg/Catch2.git "%CATCH2_DIR%"
    if %errorlevel% neq 0 (
        echo 错误：克隆Catch2库失败
        pause
        exit /b 1
    )
) else (
    echo Catch2库已存在
)

REM 编译项目
echo 编译项目...
%MSBUILD_PATH% "%PROJECT_ROOT%cpptrader.sln" /p:Configuration=Release /p:Platform=x64 /m
if %errorlevel% neq 0 (
    echo 错误：编译项目失败
    pause
    exit /b 1
)

echo 编译成功！
echo 输出文件位于：%PROJECT_ROOT%bin\x64\Release
pause
