# OrderBook 单元测试

这是一个使用 Google Test 框架为 OrderBook 类编写的单元测试项目。

## 支持平台

- Windows (Visual Studio 2022)
- Linux

## 依赖项

1. **Google Test 框架**
   - Windows: 请确保已安装 Google Test 并设置环境变量 `GTEST_INCLUDE_DIR` 和 `GTEST_LIBRARY_DIR`
   - Linux: 使用包管理器安装 `libgtest-dev`

2. **Trader 库**
   - 确保已编译并链接到 `trader.lib` (Windows) 或 `libtrader.a` (Linux)

## 构建和运行测试

### Windows 平台 (Visual Studio 2022)

1. **使用 Visual Studio IDE**:
   - 打开 `OrderBookTests.sln`
   - 选择目标配置 (Debug/Release) 和平台 (x86/x64)
   - 构建解决方案
   - 运行测试

2. **使用命令行**:
   - 打开命令提示符
   - 导航到 `gtest` 目录
   - 运行 `build.bat`
   - 查看 `compiler.log` 了解编译结果
   - 在 `../bin/Debug` 或 `../bin/Release` 目录中运行 `OrderBookTests.exe`

### Linux 平台

1. **安装依赖**:
   ```bash
   sudo apt-get install libgtest-dev cmake
   ```

2. **构建 Google Test**:
   ```bash
   cd /usr/src/gtest
   sudo cmake CMakeLists.txt
   sudo make
   sudo cp *.a /usr/lib
   ```

3. **构建单元测试**:
   ```bash
   cd /path/to/project/gtest
   mkdir build
   cd build
   cmake ..
   make
   ```

4. **运行测试**:
   ```bash
   ./OrderBookTests
   ```

## 测试内容

单元测试覆盖了以下 OrderBook 功能:

- 构造函数和基本属性
- 添加和删除限价订单
- 修改订单数量
- 添加多个相同价格的订单
- 添加不同价格的订单
- 止损订单功能
- 追踪止损订单功能
- 计算追踪止损价格
- 获取价格级别

## 项目结构

```
gtest/
├── test_order_book.cpp    # 主要测试文件
├── CMakeLists.txt         # CMake 配置文件 (Linux)
├── OrderBookTests.sln     # Visual Studio 解决方案
├── OrderBookTests.vcxproj # Visual Studio 项目文件
├── build.bat              # Windows 编译脚本
├── compiler.log           # 编译日志文件
└── README.md              # 项目说明文档
```
