@echo off
REM 编译项目
call "D:\Program\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch x64
cd build
msbuild cpptrader.sln /p:Configuration=Release /p:Platform=x64
echo 项目编译完成。
pause