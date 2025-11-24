@echo off
REM Generate Visual Studio 2022 solution
call "D:\Program\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch x64
cmake -G "Visual Studio 17 2022" -A x64 -B build .
echo Solution generated successfully. Please check the build directory.
pause