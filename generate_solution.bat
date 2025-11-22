@echo off
REM 生成Visual Studio 2022解决方案
call "D:\Program\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch x64
cmake -G "Visual Studio 17 2022" -A x64 -B build .
echo 解决方案生成完成，请查看build目录。
pause