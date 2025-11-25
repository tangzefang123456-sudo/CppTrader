#pragma once

#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include "trader/matching/order_book.h"
#include "trader/market_data/market_data_collector.h"

namespace CppTrader {
namespace SignalGenerator {

// 大单信号数据结构
typedef struct LargeOrderSignal {
    uint64_t timestamp;      // 时间戳
    std::string symbol;      // 标的代码
    double price;            // 成交价格
    int64_t volume;          // 成交量
    double amount;           // 成交额
    char direction;          // 成交方向 ('B' 买入, 'S' 卖出)
} LargeOrderSignal;

// 行情处理信号产生模块
class SignalGenerator {
public:
    SignalGenerator();
    ~SignalGenerator();

    // 禁止拷贝和移动
    SignalGenerator(const SignalGenerator&) = delete;
    SignalGenerator& operator=(const SignalGenerator&) = delete;
    SignalGenerator(SignalGenerator&&) = delete;
    SignalGenerator& operator=(SignalGenerator&&) = delete;

    // 设置大单成交额阈值（默认10万）
    void SetLargeOrderThreshold(double threshold) {
        _large_order_threshold = threshold;
    }

    // 获取当前大单成交额阈值
    double GetLargeOrderThreshold() const {
        return _large_order_threshold;
    }

    // 设置大单信号回调函数
    void SetLargeOrderSignalCallback(std::function<void(const LargeOrderSignal&)> callback) {
        _large_order_signal_callback = std::move(callback);
    }

    // 处理逐笔成交数据
    void OnTrade(const MarketData::TradeData& trade);

    // 处理逐笔委托数据
    void OnOrder(const MarketData::OrderData& order);

    // 获取指定标的的订单簿
    const Matching::OrderBook* GetOrderBook(const std::string& symbol) const;

    // 清除所有订单簿数据
    void Clear();

private:
    // 标的订单簿映射
    std::unordered_map<std::string, std::unique_ptr<Matching::OrderBook>> _order_books;
    // 大单成交额阈值
    double _large_order_threshold;
    // 大单信号回调函数
    std::function<void(const LargeOrderSignal&)> _large_order_signal_callback;

    // 获取或创建指定标的的订单簿
    Matching::OrderBook& GetOrCreateOrderBook(const std::string& symbol);
};

} // namespace SignalGenerator
} // namespace CppTrader
