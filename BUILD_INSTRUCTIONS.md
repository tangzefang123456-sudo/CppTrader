# 交易系统编译说明

本文档详细说明了如何使用Visual Studio 2022编译和运行交易系统。

## 系统要求

- Windows 10或更高版本
- Visual Studio 2022（Community、Professional或Enterprise版本）
- .NET Framework 4.8或更高版本

## 项目结构

```
d2/
├── cpptrader.sln              # Visual Studio解决方案文件
├── cpptrader.vcxproj          # 主库项目文件
├── cpptrader-tests.vcxproj    # 单元测试项目文件
├── trading_system_demo.vcxproj # 演示应用程序项目文件
├── cpptrader.props            # 项目属性表文件
├── include/                   # 头文件目录
├── source/                    # 源文件目录
├── tests/                     # 单元测试目录
├── examples/                  # 示例程序目录
└── modules/                   # 依赖库目录
```

## 准备工作

### 1. 安装Visual Studio 2022

确保您已经安装了Visual Studio 2022。如果没有安装，可以从以下链接下载：
https://visualstudio.microsoft.com/zh-hans/vs/

在安装过程中，请确保选择以下工作负载：
- C++桌面开发
- .NET桌面开发（可选，但推荐）

### 2. 克隆依赖库

交易系统依赖于CppCommon库。您需要将其克隆到modules目录中：

```bash
git clone https://github.com/chronoxor/CppCommon.git modules/cppcommon
```

如果您需要运行单元测试，还需要克隆Catch2库：

```bash
git clone https://github.com/catchorg/Catch2.git modules/Catch2
```

## 编译项目

### 1. 打开解决方案文件

双击打开`cpptrader.sln`文件，这将启动Visual Studio 2022并加载所有项目。

### 2. 选择配置和平台

在Visual Studio工具栏中，选择以下选项：
- **配置**：Debug或Release
- **平台**：x64（推荐）或x86

### 3. 编译项目

右键单击解决方案名称，然后选择"生成解决方案"。这将编译所有项目：

1. 首先编译cpptrader库
2. 然后编译cpptrader-tests单元测试
3. 最后编译trading_system_demo演示应用程序

## 运行项目

### 1. 运行单元测试

右键单击cpptrader-tests项目，选择"设为启动项目"，然后点击"运行"按钮（或按F5）。这将运行所有单元测试并显示结果。

### 2. 运行演示应用程序

右键单击trading_system_demo项目，选择"设为启动项目"，然后点击"运行"按钮（或按F5）。这将运行交易系统演示程序。

## 项目属性说明

### cpptrader.props属性表

该文件包含了所有项目的公共配置：
- 包含路径：指向头文件目录
- 库路径：指向库文件目录
- 预处理器定义：WIN32、_DEBUG或NDEBUG
- 附加依赖项：cpptrader.lib和cppcommon.lib
- C++标准：C++20
- 平台工具集：v143（Visual Studio 2022）

### 项目依赖关系

- cpptrader-tests依赖于cpptrader
- trading_system_demo依赖于cpptrader

## 常见问题

### 1. 找不到cppcommon库

确保您已经将CppCommon库克隆到modules/cppcommon目录中。如果仍然找不到，请检查cpptrader.props文件中的CppCommonRoot路径是否正确。

### 2. 编译错误：找不到头文件

检查cpptrader.props文件中的IncludePath是否包含了所有必要的头文件目录。

### 3. 链接错误：找不到库文件

检查cpptrader.props文件中的LibraryPath是否包含了所有必要的库文件目录。确保cpptrader.lib和cppcommon.lib已经被编译并放置在正确的目录中。

### 4. 运行时错误：找不到DLL文件

确保cppcommon.dll文件位于系统路径中，或者与可执行文件放在同一目录中。

## 调试技巧

### 1. 设置断点

在Visual Studio中，您可以在代码中设置断点，以便在运行时暂停程序并检查变量的值。

### 2. 查看输出

在"输出"窗口中，您可以查看程序的输出信息，包括调试信息和错误信息。

### 3. 使用调试器

Visual Studio提供了强大的调试器，您可以使用它来单步执行代码、查看调用栈、检查内存等。

## 性能优化

### 1. 使用Release配置

Release配置启用了优化选项，可以提高程序的运行速度。

### 2. 使用x64平台

x64平台可以利用更多的内存和处理器资源，提高程序的性能。

### 3. 优化代码

您可以使用Visual Studio的性能分析工具来找出程序中的性能瓶颈，并进行优化。

## 发布程序

### 1. 编译Release版本

选择Release配置，然后编译解决方案。

### 2. 收集必要的文件

- 可执行文件（.exe）
- 依赖的DLL文件（如cppcommon.dll）
- 配置文件（如果有的话）

### 3. 部署程序

将收集到的文件复制到目标计算机上，并确保目标计算机上已经安装了必要的运行时库。

## 联系方式

如果您在使用过程中遇到问题，请随时联系我们。
