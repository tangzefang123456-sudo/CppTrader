#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include "trader/market_data/market_data_collector.h"
#include "trader/signal_generator/signal_generator.h"
#include "trader/execution/execution_manager.h"

using namespace CppTrader;

// 示例行情收取器实现
class ExampleMarketDataCollector : public MarketData::MarketDataCollector {
public:
    ExampleMarketDataCollector() : _is_running(false) {}

    bool Start() override {
        if (_is_running) {
            return true;
        }

        _is_running = true;
        _thread = std::thread([this]() {
            Run();
        });
        return true;
    }

    bool Stop() override {
        if (!_is_running) {
            return true;
        }

        _is_running = false;
        if (_thread.joinable()) {
            _thread.join();
        }
        return true;
    }

    bool IsRunning() const override {
        return _is_running;
    }

private:
    bool _is_running;
    std::thread _thread;

    void Run() {
        uint64_t timestamp = 0;
        while (_is_running) {
            // 模拟生成逐笔成交数据
            MarketData::TradeData trade;
            trade.timestamp = timestamp++;
            trade.symbol = "AAPL";
            trade.price = 150.0 + (timestamp % 100) * 0.1;
            trade.volume = 100 + (timestamp % 10) * 10;
            trade.direction = (timestamp % 2 == 0) ? 'B' : 'S';
            OnTrade(trade);

            // 模拟生成逐笔委托数据
            MarketData::OrderData order;
            order.timestamp = timestamp++;
            order.symbol = "AAPL";
            order.order_id = timestamp;
            order.type = (timestamp % 2 == 0) ? 'B' : 'S';
            order.price = 150.0 + (timestamp % 100) * 0.1;
            order.volume = 100 + (timestamp % 10) * 10;
            order.status = 'P';
            OnOrder(order);

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
};

// 示例执行处理器实现
class ExampleExecutionHandler : public Execution::ExecutionHandler {
public:
    uint64_t SubmitOrder(const Execution::OrderRequest& request) override {
        static uint64_t next_order_id = 1;
        uint64_t order_id = next_order_id++;

        // 模拟委托执行结果
        std::thread([this, order_id, request]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            Execution::OrderResult result;
            result.order_id = order_id;
            result.symbol = request.symbol;
            result.side = request.side;
            result.price = request.price;
            result.quantity = request.quantity;
            result.filled_quantity = request.quantity;
            result.filled_amount = request.price * request.quantity;
            result.status = 'F';

            OnOrderResult(result);
        }).detach();

        return order_id;
    }

    bool CancelOrder(uint64_t order_id) override {
        // 模拟撤销委托
        return true;
    }

    bool QueryOrder(uint64_t order_id, Execution::OrderResult& result) override {
        // 模拟查询委托状态
        return false;
    }
};

int main() {
    std::cout << "Trading System Demo" << std::endl;
    std::cout << "==================" << std::endl;

    // 创建行情收取器
    auto market_data_collector = std::make_unique<ExampleMarketDataCollector>();

    // 创建信号产生器
    auto signal_generator = std::make_unique<SignalGenerator::SignalGenerator>();
    signal_generator->SetLargeOrderThreshold(100000.0); // 10万阈值

    // 创建执行处理器和执行管理器
    auto execution_handler = std::make_unique<ExampleExecutionHandler>();
    auto execution_manager = std::make_unique<Execution::ExecutionManager>(std::move(execution_handler));

    // 设置委托结果回调
    execution_handler->SetOrderResultCallback([](const Execution::OrderResult& result) {
        std::cout << "Order executed: ID=" << result.order_id << ", Symbol=" << result.symbol << ", Side=" << result.side << ", FilledQuantity=" << result.filled_quantity << ", FilledAmount=" << result.filled_amount << std::endl;
    });

    // 连接行情收取器到信号产生器
    market_data_collector->SetTradeCallback([&](const MarketData::TradeData& trade) {
        double amount = trade.price * trade.volume;
        std::cout << "Trade: Symbol=" << trade.symbol << ", Price=" << trade.price << ", Volume=" << trade.volume << ", Amount=" << amount << ", Direction=" << trade.direction << std::endl;
        signal_generator->OnTrade(trade);
    });

    market_data_collector->SetOrderCallback([&](const MarketData::OrderData& order) {
        std::cout << "Order: Symbol=" << order.symbol << ", OrderID=" << order.order_id << ", Type=" << order.type << ", Price=" << order.price << ", Volume=" << order.volume << ", Status=" << order.status << std::endl;
        signal_generator->OnOrder(order);
    });

    // 连接信号产生器到执行管理器
    signal_generator->SetLargeOrderSignalCallback([&](const SignalGenerator::LargeOrderSignal& signal) {
        std::cout << "Large order signal: Symbol=" << signal.symbol << ", Price=" << signal.price << ", Volume=" << signal.volume << ", Amount=" << signal.amount << ", Direction=" << signal.direction << std::endl;
        execution_manager->OnLargeOrderSignal(signal);
    });

    // 启动行情收取器
    std::cout << "Starting market data collector..." << std::endl;
    market_data_collector->Start();

    // 运行一段时间
    std::cout << "Running for 10 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // 停止行情收取器
    std::cout << "Stopping market data collector..." << std::endl;
    market_data_collector->Stop();

    std::cout << "Demo completed." << std::endl;

    return 0;
}
