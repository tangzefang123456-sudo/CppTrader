@echo off

:: 编译Visual Studio 2022解决方案
call "%VS170COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat" x64

:: 创建编译日志文件
echo 编译开始时间: %date% %time% > compiler.log

echo 正在编译CircularQueue解决方案... >> compiler.log
msbuild CircularQueue.sln /p:Configuration=Debug /p:Platform=x64 /m >> compiler.log 2>&1

if %ERRORLEVEL% equ 0 (
    echo 编译成功！ >> compiler.log
    echo 编译完成时间: %date% %time% >> compiler.log
    echo 编译成功，请查看compiler.log文件获取详细信息。
) else (
    echo 编译失败！ >> compiler.log
    echo 编译完成时间: %date% %time% >> compiler.log
    echo 编译失败，请查看compiler.log文件获取详细信息。
)

pause
