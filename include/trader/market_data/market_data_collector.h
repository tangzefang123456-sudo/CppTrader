#pragma once

#include <string>
#include <memory>
#include <functional>

namespace CppTrader {
namespace MarketData {

// 逐笔成交数据结构
typedef struct TradeData {
    uint64_t timestamp;      // 时间戳
    std::string symbol;      // 标的代码
    double price;            // 成交价格
    int64_t volume;          // 成交量
    char direction;          // 成交方向 ('B' 买入, 'S' 卖出)
} TradeData;

// 逐笔委托数据结构
typedef struct OrderData {
    uint64_t timestamp;      // 时间戳
    std::string symbol;      // 标的代码
    uint64_t order_id;       // 委托编号
    char type;               // 委托类型 ('B' 买入, 'S' 卖出)
    double price;            // 委托价格
    int64_t volume;          // 委托数量
    char status;             // 委托状态 ('P' 已挂单, 'C' 已成交, 'X' 已撤销)
} OrderData;

// 行情收取抽象类
class MarketDataCollector {
public:
    MarketDataCollector() = default;
    virtual ~MarketDataCollector() = default;

    // 禁止拷贝和移动
    MarketDataCollector(const MarketDataCollector&) = delete;
    MarketDataCollector& operator=(const MarketDataCollector&) = delete;
    MarketDataCollector(MarketDataCollector&&) = delete;
    MarketDataCollector& operator=(MarketDataCollector&&) = delete;

    // 启动行情收取
    virtual bool Start() = 0;
    // 停止行情收取
    virtual bool Stop() = 0;
    // 检查是否正在运行
    virtual bool IsRunning() const = 0;

    // 设置逐笔成交回调函数
    void SetTradeCallback(std::function<void(const TradeData&)> callback) {
        _trade_callback = std::move(callback);
    }

    // 设置逐笔委托回调函数
    void SetOrderCallback(std::function<void(const OrderData&)> callback) {
        _order_callback = std::move(callback);
    }

protected:
    // 触发逐笔成交回调
    void OnTrade(const TradeData& trade) {
        if (_trade_callback) {
            _trade_callback(trade);
        }
    }

    // 触发逐笔委托回调
    void OnOrder(const OrderData& order) {
        if (_order_callback) {
            _order_callback(order);
        }
    }

private:
    std::function<void(const TradeData&)> _trade_callback;
    std::function<void(const OrderData&)> _order_callback;
};

// 行情刻录器类
class MarketDataRecorder {
public:
    MarketDataRecorder(const std::string& file_path);
    ~MarketDataRecorder();

    // 禁止拷贝和移动
    MarketDataRecorder(const MarketDataRecorder&) = delete;
    MarketDataRecorder& operator=(const MarketDataRecorder&) = delete;
    MarketDataRecorder(MarketDataRecorder&&) = delete;
    MarketDataRecorder& operator=(MarketDataRecorder&&) = delete;

    // 打开刻录文件
    bool Open();
    // 关闭刻录文件
    bool Close();
    // 检查文件是否打开
    bool IsOpen() const;

    // 刻录逐笔成交数据
    bool RecordTrade(const TradeData& trade);
    // 刻录逐笔委托数据
    bool RecordOrder(const OrderData& order);

private:
    std::string _file_path;
    void* _file_handle; // 实际应该使用平台相关的文件句柄类型
};

// 行情播放器类
class MarketDataPlayer {
public:
    MarketDataPlayer(const std::string& file_path);
    ~MarketDataPlayer();

    // 禁止拷贝和移动
    MarketDataPlayer(const MarketDataPlayer&) = delete;
    MarketDataPlayer& operator=(const MarketDataPlayer&) = delete;
    MarketDataPlayer(MarketDataPlayer&&) = delete;
    MarketDataPlayer& operator=(MarketDataPlayer&&) = delete;

    // 打开播放文件
    bool Open();
    // 关闭播放文件
    bool Close();
    // 检查文件是否打开
    bool IsOpen() const;

    // 设置逐笔成交回调函数
    void SetTradeCallback(std::function<void(const TradeData&)> callback) {
        _trade_callback = std::move(callback);
    }

    // 设置逐笔委托回调函数
    void SetOrderCallback(std::function<void(const OrderData&)> callback) {
        _order_callback = std::move(callback);
    }

    // 播放行情数据
    bool Play(double speed = 1.0);
    // 暂停播放
    bool Pause();
    // 继续播放
    bool Resume();
    // 停止播放
    bool Stop();

private:
    std::string _file_path;
    void* _file_handle; // 实际应该使用平台相关的文件句柄类型
    bool _is_playing;
    bool _is_paused;
    std::function<void(const TradeData&)> _trade_callback;
    std::function<void(const OrderData&)> _order_callback;
};

} // namespace MarketData
} // namespace CppTrader
