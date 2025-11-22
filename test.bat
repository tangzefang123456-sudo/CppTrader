@echo off
setlocal enabledelayedexpansion

REM 设置项目根目录
set PROJECT_ROOT=%~dp0
set BIN_DIR=%PROJECT_ROOT%bin\x64\Release

REM 检查测试程序是否存在
if not exist "%BIN_DIR%\cpptrader-tests.exe" (
    echo 错误：测试程序不存在，请先编译项目
    pause
    exit /b 1
)

REM 运行单元测试
echo 运行单元测试...
"%BIN_DIR%\cpptrader-tests.exe"
if %errorlevel% neq 0 (
    echo 错误：单元测试失败
    pause
    exit /b 1
)

echo 单元测试成功！

REM 检查演示程序是否存在
if not exist "%BIN_DIR%\trading_system_demo.exe" (
    echo 错误：演示程序不存在，请先编译项目
    pause
    exit /b 1
)

REM 运行演示程序
echo 运行演示程序...
"%BIN_DIR%\trading_system_demo.exe"
if %errorlevel% neq 0 (
    echo 错误：演示程序运行失败
    pause
    exit /b 1
)

echo 演示程序运行成功！
pause
