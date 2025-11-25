#ifndef TRADING_EXECUTOR_H
#define TRADING_EXECUTOR_H

#include "market_data_processor.h"
#include <string>
#include <vector>

namespace TradingSystem {

// 订单数据结构
typedef struct {
    uint64_t timestamp;      // 时间戳（纳秒）
    std::string symbol;      // 标的代码
    char order_type;         // 订单类型（'B'表示买入，'S'表示卖出）
    double price;            // 订单价格
    int64_t volume;          // 订单数量
    std::string order_id;    // 订单ID
    char status;             // 订单状态（'P'表示已提交，'F'表示已成交，'C'表示已撤销）
} Order;

// 交易执行模块
class TradingExecutor {
public:
    TradingExecutor();
    virtual ~TradingExecutor() = default;

    // 初始化交易执行模块
    bool Init(const std::string& config);

    // 处理大单信号
    void ProcessLargeOrderSignal(const LargeOrderSignal& signal);

    // 获取所有订单
    std::vector<Order> GetAllOrders() const;

    // 获取指定订单ID的订单
    Order GetOrderById(const std::string& order_id) const;

    // 撤销指定订单ID的订单
    bool CancelOrder(const std::string& order_id);

    // 清空所有订单
    void Clear();

private:
    // 生成订单ID
    std::string GenerateOrderId();

    // 执行本方价追随策略
    void ExecuteSameSidePriceFollowingStrategy(const LargeOrderSignal& signal);

private:
    std::vector<Order> orders_;  // 所有订单
    int64_t order_id_counter_;   // 订单ID计数器
    const double PRICE_FOLLOWING_OFFSET = 0.01;  // 价格追随偏移量（0.01元）
    const int64_t ORDER_VOLUME = 100;            // 每次下单数量（100股）
};

} // namespace TradingSystem

#endif // TRADING_EXECUTOR_H
