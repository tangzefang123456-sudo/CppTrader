@echo off
setlocal

REM 创建构建目录
if not exist "build" mkdir build
cd build

REM 生成Visual Studio 2022解决方案
cmake -G "Visual Studio 17 2022" -A x64 ..

REM 编译项目
msbuild MemoryPool.sln /p:Configuration=Debug /p:Platform=x64 > ..\compiler.log 2>&1

REM 检查编译结果
if %errorlevel% equ 0 (
    echo 编译成功！结果已写入compiler.log
    echo 运行单元测试：
    bin\Debug\x64\MemoryPoolTest.exe
) else (
    echo 编译失败！详细信息请查看compiler.log
)

cd ..
pause
