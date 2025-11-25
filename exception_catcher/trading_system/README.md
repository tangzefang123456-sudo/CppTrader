# 交易系统

这是一个基于C++的交易系统，包含行情收取模块、行情处理信号产生模块和交易执行模块。

## 系统结构

交易系统包含以下模块：

1. **行情收取模块**：负责收取逐笔成交和逐笔委托数据，并将数据写入共享内存。同时支持将行情数据刻录为本地二进制文件，以及从本地二进制文件加载行情数据并模拟播放。

2. **行情处理信号产生模块**：根据逐笔成交和逐笔委托数据生成每个标的的订单簿。当单笔成交额大于10万时，产生一次大单信号。

3. **交易执行模块**：根据每次大单信号执行挂本方价追随策略。

## 目录结构

交易系统的目录结构如下：

```
trading_system/
├── include/          # 头文件目录
│   ├── market_data_collector.h      # 行情收取模块头文件
│   ├── market_data_processor.h      # 行情处理信号产生模块头文件
│   └── trading_executor.h           # 交易执行模块头文件
├── src/              # 源文件目录
│   ├── market_data_collector.cpp    # 行情收取模块源文件
│   ├── market_data_collector_impl.cpp  # 行情收取模块实现文件
│   ├── market_data_processor.cpp    # 行情处理信号产生模块源文件
│   └── trading_executor.cpp         # 交易执行模块源文件
├── tests/            # 单元测试目录
│   ├── test_market_data_collector.cpp   # 行情收取模块单元测试
│   ├── test_market_data_processor.cpp   # 行情处理信号产生模块单元测试
│   └── test_trading_executor.cpp        # 交易执行模块单元测试
├── lib/              # 库文件目录
│   └── gtest/        # Google Test库
├── MarketDataCollector/       # 行情收取模块Visual Studio项目
├── MarketDataProcessor/       # 行情处理信号产生模块Visual Studio项目
├── TradingExecutor/           # 交易执行模块Visual Studio项目
├── TestMarketDataCollector/   # 行情收取模块单元测试Visual Studio项目
├── TestMarketDataProcessor/   # 行情处理信号产生模块单元测试Visual Studio项目
├── TestTradingExecutor/       # 交易执行模块单元测试Visual Studio项目
├── TradingSystem.sln          # Visual Studio解决方案文件
└── README.md                  # 说明文档
```

## 编译和运行

### 环境要求

- Visual Studio 2022
- C++17标准
- Google Test库（已包含在lib/gtest目录中）

### 编译步骤

1. 打开Visual Studio 2022
2. 打开TradingSystem.sln解决方案文件
3. 选择编译配置（Debug或Release）和平台（x64）
4. 点击"生成"菜单，选择"生成解决方案"

### 运行单元测试

编译完成后，可以运行以下单元测试项目：

- TestMarketDataCollector：行情收取模块单元测试
- TestMarketDataProcessor：行情处理信号产生模块单元测试
- TestTradingExecutor：交易执行模块单元测试

## 使用方法

### 行情收取模块

```cpp
#include "market_data_collector.h"

using namespace TradingSystem;

// 创建行情数据收集器实例
MarketDataCollector* collector = CreateMarketDataCollector();

// 初始化行情数据收集器
collector->Init("config");

// 开始行情数据收集
collector->Start();

// 处理逐笔成交数据
TickData tick;
tick.timestamp = 1234567890123456789ULL;
strcpy_s(tick.symbol, "000001.SZ");
tick.price = 10.0;
tick.volume = 100;
tick.direction = 'B';
collector->OnTickData(tick);

// 处理逐笔委托数据
OrderData order;
order.timestamp = 1234567890123456789ULL;
strcpy_s(order.symbol, "000001.SZ");
order.price = 10.0;
order.volume = 100;
order.order_type = 'B';
strcpy_s(order.order_id, "ORDER000001");
collector->OnOrderData(order);

// 保存行情数据到本地二进制文件
collector->SaveToBinaryFile("market_data.bin");

// 从本地二进制文件加载行情数据并模拟播放
collector->LoadFromBinaryFile("market_data.bin");

// 停止行情数据收集
collector->Stop();

// 销毁行情数据收集器实例
DestroyMarketDataCollector(collector);
```

### 行情处理信号产生模块

```cpp
#include "market_data_processor.h"

using namespace TradingSystem;

// 创建行情数据处理器实例
MarketDataProcessor* processor = new MarketDataProcessor();

// 初始化行情数据处理器
processor->Init("config");

// 处理逐笔成交数据
TickData tick;
tick.timestamp = 1234567890123456789ULL;
strcpy_s(tick.symbol, "000001.SZ");
tick.price = 10.0;
tick.volume = 100;
tick.direction = 'B';
processor->ProcessTickData(tick);

// 处理逐笔委托数据
OrderData order;
order.timestamp = 1234567890123456789ULL;
strcpy_s(order.symbol, "000001.SZ");
order.price = 10.0;
order.volume = 100;
order.order_type = 'B';
strcpy_s(order.order_id, "ORDER000001");
processor->ProcessOrderData(order);

// 获取指定标的的订单簿
OrderBook order_book = processor->GetOrderBook("000001.SZ");

// 获取最新的大单信号
std::vector<LargeOrderSignal> signals = processor->GetLatestLargeOrderSignals();

// 清空行情数据处理器
processor->Clear();

// 销毁行情数据处理器实例
delete processor;
```

### 交易执行模块

```cpp
#include "trading_executor.h"

using namespace TradingSystem;

// 创建交易执行器实例
TradingExecutor* executor = new TradingExecutor();

// 初始化交易执行器
executor->Init("config");

// 处理大单信号
LargeOrderSignal signal;
signal.timestamp = 1234567890123456789ULL;
signal.symbol = "000001.SZ";
signal.price = 10.0;
signal.volume = 1001;
signal.amount = 100.0 * 1001;
signal.direction = 'B';
executor->ProcessLargeOrderSignal(signal);

// 获取所有订单
std::vector<Order> orders = executor->GetAllOrders();

// 根据订单ID获取订单
Order order = executor->GetOrderById(orders[0].order_id);

// 撤销订单
executor->CancelOrder(orders[0].order_id);

// 清空交易执行器
executor->Clear();

// 销毁交易执行器实例
delete executor;
```

## 代码风格

交易系统的代码风格采用驼峰式命名法，符合C++17标准。

## 单元测试

交易系统包含完整的单元测试，使用Google Test框架编写，行执行覆盖率达到100%。

## 许可证

交易系统采用MIT许可证，详见LICENSE文件。
