#ifndef MARKET_DATA_COLLECTOR_H
#define MARKET_DATA_COLLECTOR_H

#include <string>
#include <vector>
#include <fstream>

namespace TradingSystem {

// 逐笔成交数据结构
typedef struct {
    uint64_t timestamp;      // 时间戳（纳秒）
    std::string symbol;      // 标的代码
    double price;            // 成交价格
    int64_t volume;          // 成交数量
    char direction;          // 成交方向（'B'表示买入，'S'表示卖出）
} TickData;

// 逐笔委托数据结构
typedef struct {
    uint64_t timestamp;      // 时间戳（纳秒）
    std::string symbol;      // 标的代码
    char order_type;         // 委托类型（'B'表示买入，'S'表示卖出）
    double price;            // 委托价格
    int64_t volume;          // 委托数量
    uint64_t order_id;       // 委托ID
} OrderData;

// 行情收取抽象类
class MarketDataCollector {
public:
    MarketDataCollector() = default;
    virtual ~MarketDataCollector() = default;

    // 初始化行情收取模块
    virtual bool Init(const std::string& config) = 0;

    // 启动行情收取
    virtual bool Start() = 0;

    // 停止行情收取
    virtual bool Stop() = 0;

    // 获取最新的逐笔成交数据
    virtual std::vector<TickData> GetLatestTicks() = 0;

    // 获取最新的逐笔委托数据
    virtual std::vector<OrderData> GetLatestOrders() = 0;

    // 刻录行情数据到本地二进制文件
    virtual bool SaveToBinaryFile(const std::string& file_path);

    // 从本地二进制文件加载行情数据并模拟播放
    virtual bool LoadFromBinaryFile(const std::string& file_path);

    // 处理逐笔成交数据
    virtual void OnTickData(const TickData& tick) = 0;

    // 处理逐笔委托数据
    virtual void OnOrderData(const OrderData& order) = 0;

protected:
    std::vector<TickData> ticks_;     // 逐笔成交数据缓存
    std::vector<OrderData> orders_;   // 逐笔委托数据缓存
};

} // namespace TradingSystem

#endif // MARKET_DATA_COLLECTOR_H
