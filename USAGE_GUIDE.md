# 交易系统使用指南

本文档详细说明了如何使用交易系统进行市场数据处理、信号生成和交易执行。

## 系统概述

交易系统由三个核心模块组成：
1. **市场数据模块**：负责接收和处理市场数据
2. **信号生成模块**：负责根据市场数据生成交易信号
3. **执行模块**：负责根据交易信号执行交易

## 快速开始

### 1. 编译项目

首先，您需要编译项目。您可以使用Visual Studio 2022打开`cpptrader.sln`文件，然后选择Release配置和x64平台，最后点击"生成解决方案"。

或者，您可以使用提供的`build.bat`批处理文件自动编译项目：

```bash
build.bat
```

### 2. 运行演示程序

编译成功后，您可以运行`trading_system_demo.exe`演示程序。该程序位于`bin\x64\Release`目录中。

演示程序将模拟市场数据，并根据预设的策略生成交易信号和执行交易。

## 核心组件说明

### 1. 市场数据模块

市场数据模块负责接收和处理市场数据。它支持以下功能：

- 接收实时市场数据
- 处理历史市场数据
- 维护订单簿
- 提供市场数据查询接口

#### 使用示例

```cpp
// 创建市场管理器
CppTrader::ITCH::MarketManager market_manager;

// 创建订单簿
CppTrader::ITCH::OrderBook order_book;

// 注册订单簿更新回调
market_manager.OnOrderBookUpdate([&](const CppTrader::ITCH::OrderBookUpdate& update) {
    // 更新订单簿
    order_book.Update(update);
});

// 开始接收市场数据
market_manager.Start();
```

### 2. 信号生成模块

信号生成模块负责根据市场数据生成交易信号。它支持以下功能：

- 大单信号检测
- 价格波动信号检测
- 成交量信号检测
- 自定义信号生成

#### 使用示例

```cpp
// 创建信号生成器
CppTrader::ITCH::SignalGenerator signal_generator;

// 注册信号回调
signal_generator.OnSignal([&](const CppTrader::ITCH::Signal& signal) {
    // 处理交易信号
    if (signal.Type == CppTrader::ITCH::SignalType::BIG_TRADE) {
        // 大单信号处理逻辑
    }
});

// 开始生成信号
signal_generator.Start();
```

### 3. 执行模块

执行模块负责根据交易信号执行交易。它支持以下功能：

- 挂本方价追随策略
- 自定义交易策略
- 交易执行监控
- 交易结果反馈

#### 使用示例

```cpp
// 创建执行管理器
CppTrader::ITCH::ExecutionManager execution_manager;

// 注册执行结果回调
execution_manager.OnExecutionResult([&](const CppTrader::ITCH::ExecutionResult& result) {
    // 处理执行结果
    if (result.Success) {
        // 交易成功处理逻辑
    } else {
        // 交易失败处理逻辑
    }
});

// 开始执行交易
execution_manager.Start();
```

## 策略配置

### 1. 大单信号策略

您可以配置大单信号的阈值：

```cpp
// 设置大单阈值为100000美元
signal_generator.SetBigTradeThreshold(100000);
```

### 2. 挂本方价追随策略

您可以配置挂本方价追随策略的参数：

```cpp
// 设置挂本方价追随策略
execution_manager.SetStrategy(std::make_shared<CppTrader::ITCH::FollowBidAskStrategy>());
```

## 数据格式

### 1. 市场数据格式

市场数据采用NASDAQ ITCH 5.0格式。主要包含以下字段：

- 股票代码
- 价格
- 数量
- 时间戳
- 交易类型

### 2. 交易信号格式

交易信号包含以下字段：

- 信号类型
- 股票代码
- 价格
- 数量
- 时间戳

### 3. 执行结果格式

执行结果包含以下字段：

- 交易ID
- 股票代码
- 价格
- 数量
- 时间戳
- 成功标志
- 错误信息

## 性能优化

### 1. 使用多线程

交易系统支持多线程处理，可以提高性能：

```cpp
// 设置线程数为4
market_manager.SetThreads(4);
signal_generator.SetThreads(4);
execution_manager.SetThreads(4);
```

### 2. 使用内存池

交易系统使用内存池来减少内存分配和释放的开销：

```cpp
// 设置内存池大小为1024*1024字节
CppTrader::Memory::MemoryPool::SetSize(1024*1024);
```

### 3. 使用零拷贝

交易系统使用零拷贝技术来减少数据复制的开销：

```cpp
// 启用零拷贝
market_manager.SetZeroCopy(true);
```

## 监控和日志

### 1. 监控

交易系统提供监控接口，可以监控系统的运行状态：

```cpp
// 获取系统状态
CppTrader::ITCH::SystemStatus status = market_manager.GetStatus();

// 打印系统状态
std::cout << "系统状态：" << status.ToString() << std::endl;
```

### 2. 日志

交易系统提供日志接口，可以记录系统的运行日志：

```cpp
// 设置日志级别为DEBUG
CppTrader::Logging::Logger::SetLevel(CppTrader::Logging::Level::DEBUG);

// 记录日志
CppTrader::Logging::Logger::Debug("调试信息");
CppTrader::Logging::Logger::Info("普通信息");
CppTrader::Logging::Logger::Warning("警告信息");
CppTrader::Logging::Logger::Error("错误信息");
CppTrader::Logging::Logger::Fatal("致命错误");
```

## 常见问题

### 1. 如何添加新的交易策略？

您可以继承`CppTrader::ITCH::Strategy`类，实现自己的交易策略：

```cpp
class MyStrategy : public CppTrader::ITCH::Strategy {
public:
    void OnSignal(const CppTrader::ITCH::Signal& signal) override {
        // 实现自己的交易逻辑
    }
};
```

然后，将新的策略设置到执行管理器中：

```cpp
execution_manager.SetStrategy(std::make_shared<MyStrategy>());
```

### 2. 如何添加新的信号类型？

您可以在`CppTrader::ITCH::SignalType`枚举中添加新的信号类型：

```cpp
enum class SignalType {
    BIG_TRADE,
    PRICE_VOLATILITY,
    VOLUME_SPIKE,
    MY_NEW_SIGNAL // 新的信号类型
};
```

然后，在信号生成器中生成新的信号类型：

```cpp
signal_generator.GenerateSignal(SignalType::MY_NEW_SIGNAL, "AAPL", 150.0, 100, timestamp);
```

### 3. 如何处理异常？

交易系统使用异常来处理错误。您可以使用try-catch块来捕获异常：

```cpp
try {
    // 交易系统代码
} catch (const std::exception& e) {
    // 处理异常
    std::cout << "异常：" << e.what() << std::endl;
}
```

## 联系方式

如果您在使用过程中遇到问题，请随时联系我们。
