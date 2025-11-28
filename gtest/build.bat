@echo off

REM 编译脚本，用于使用Visual Studio 2022编译OrderBook单元测试
REM 编译结果将写入compiler.log文件

echo 编译开始于: %date% %time% > compiler.log
echo =========================================== >> compiler.log
echo. >> compiler.log

REM 编译项目
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" OrderBookTests.vcxproj /p:Configuration=Debug /p:Platform=x64 >> compiler.log 2>&1

REM 检查编译结果
if %errorlevel% equ 0 (
    echo 编译成功! >> compiler.log
) else (
    echo 编译失败! >> compiler.log
)

echo. >> compiler.log
echo =========================================== >> compiler.log
echo 编译结束于: %date% %time% >> compiler.log

echo 编译完成。请查看compiler.log文件获取详细信息。
pause