#ifndef MARKET_DATA_PROCESSOR_H
#define MARKET_DATA_PROCESSOR_H

#include "market_data_collector.h"
#include <unordered_map>
#include <queue>
#include <functional>

namespace TradingSystem {

// 订单簿条目数据结构
typedef struct {
    double price;            // 价格
    int64_t volume;          // 数量
} OrderBookEntry;

// 订单簿数据结构
typedef struct {
    std::priority_queue<OrderBookEntry, std::vector<OrderBookEntry>, 
                       std::function<bool(const OrderBookEntry&, const OrderBookEntry&)>> bids;  // 买单队列（价格从高到低）
    std::priority_queue<OrderBookEntry, std::vector<OrderBookEntry>, 
                       std::function<bool(const OrderBookEntry&, const OrderBookEntry&)>> asks;  // 卖单队列（价格从低到高）
} OrderBook;

// 大单信号数据结构
typedef struct {
    uint64_t timestamp;      // 时间戳（纳秒）
    std::string symbol;      // 标的代码
    double price;            // 成交价格
    int64_t volume;          // 成交数量
    double amount;           // 成交金额
    char direction;          // 成交方向（'B'表示买入，'S'表示卖出）
} LargeOrderSignal;

// 行情处理信号产生模块
class MarketDataProcessor {
public:
    MarketDataProcessor();
    virtual ~MarketDataProcessor() = default;

    // 初始化行情处理模块
    bool Init(const std::string& config);

    // 处理逐笔成交数据
    void ProcessTickData(const TickData& tick);

    // 处理逐笔委托数据
    void ProcessOrderData(const OrderData& order);

    // 获取指定标的的订单簿
    OrderBook GetOrderBook(const std::string& symbol) const;

    // 获取最新的大单信号
    std::vector<LargeOrderSignal> GetLatestLargeOrderSignals();

    // 清空所有数据
    void Clear();

private:
    // 生成大单信号
    void GenerateLargeOrderSignal(const TickData& tick);

private:
    std::unordered_map<std::string, OrderBook> order_books_;  // 标的代码到订单簿的映射
    std::vector<LargeOrderSignal> large_order_signals_;       // 大单信号缓存
    const double LARGE_ORDER_THRESHOLD = 100000.0;            // 大单阈值（10万）
};

} // namespace TradingSystem

#endif // MARKET_DATA_PROCESSOR_H
