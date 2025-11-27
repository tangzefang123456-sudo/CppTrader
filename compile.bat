@echo off
setlocal

REM 创建构建目录
if not exist "build" mkdir build
cd build

REM 生成Visual Studio 2022解决方案
cmake -G "Visual Studio 17 2022" -A x64 ..

REM 编译项目
msbuild MemoryPool.sln /p:Configuration=Debug /p:Platform=x64 > ..\compiler.log 2>&1

cd ..
