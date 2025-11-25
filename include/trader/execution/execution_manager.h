#pragma once

#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include "trader/market_data/market_data_collector.h"
#include "trader/signal_generator/signal_generator.h"

namespace CppTrader {
namespace Execution {

// 委托订单数据结构
typedef struct OrderRequest {
    std::string symbol;      // 标的代码
    char side;               // 买卖方向 ('B' 买入, 'S' 卖出)
    double price;            // 委托价格
    int64_t quantity;        // 委托数量
} OrderRequest;

// 委托执行结果数据结构
typedef struct OrderResult {
    uint64_t order_id;       // 委托编号
    std::string symbol;      // 标的代码
    char side;               // 买卖方向 ('B' 买入, 'S' 卖出)
    double price;            // 委托价格
    int64_t quantity;        // 委托数量
    int64_t filled_quantity; // 成交数量
    double filled_amount;    // 成交金额
    char status;             // 委托状态 ('P' 已挂单, 'F' 已成交, 'C' 已撤销, 'R' 已拒绝)
} OrderResult;

// 交易执行抽象类
class ExecutionHandler {
public:
    ExecutionHandler() = default;
    virtual ~ExecutionHandler() = default;

    // 禁止拷贝和移动
    ExecutionHandler(const ExecutionHandler&) = delete;
    ExecutionHandler& operator=(const ExecutionHandler&) = delete;
    ExecutionHandler(ExecutionHandler&&) = delete;
    ExecutionHandler& operator=(ExecutionHandler&&) = delete;

    // 提交委托订单
    virtual uint64_t SubmitOrder(const OrderRequest& request) = 0;
    // 撤销委托订单
    virtual bool CancelOrder(uint64_t order_id) = 0;
    // 查询委托订单状态
    virtual bool QueryOrder(uint64_t order_id, OrderResult& result) = 0;

    // 设置委托结果回调函数
    void SetOrderResultCallback(std::function<void(const OrderResult&)> callback) {
        _order_result_callback = std::move(callback);
    }

protected:
    // 触发委托结果回调
    void OnOrderResult(const OrderResult& result) {
        if (_order_result_callback) {
            _order_result_callback(result);
        }
    }

private:
    std::function<void(const OrderResult&)> _order_result_callback;
};

// 交易执行管理器
class ExecutionManager {
public:
    ExecutionManager(std::unique_ptr<ExecutionHandler> execution_handler);
    ~ExecutionManager();

    // 禁止拷贝和移动
    ExecutionManager(const ExecutionManager&) = delete;
    ExecutionManager& operator=(const ExecutionManager&) = delete;
    ExecutionManager(ExecutionManager&&) = delete;
    ExecutionManager& operator=(ExecutionManager&&) = delete;

    // 设置大单信号回调（用于接收信号产生模块的信号）
    void OnLargeOrderSignal(const SignalGenerator::LargeOrderSignal& signal);

    // 设置行情数据回调（用于获取最新行情）
    void OnTrade(const MarketData::TradeData& trade);
    void OnOrder(const MarketData::OrderData& order);

    // 设置追随策略参数
    void SetFollowStrategyParams(double price_offset, int64_t quantity) {
        _price_offset = price_offset;
        _follow_quantity = quantity;
    }

    // 获取当前追随策略参数
    void GetFollowStrategyParams(double& price_offset, int64_t& quantity) const {
        price_offset = _price_offset;
        quantity = _follow_quantity;
    }

private:
    std::unique_ptr<ExecutionHandler> _execution_handler;
    // 追随策略参数
    double _price_offset;     // 价格偏移量
    int64_t _follow_quantity; // 追随委托数量

    // 根据大单信号执行追随策略
    void ExecuteFollowStrategy(const SignalGenerator::LargeOrderSignal& signal);
};

} // namespace Execution
} // namespace CppTrader
