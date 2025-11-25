#include "trading_executor.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace TradingSystem {

TradingExecutor::TradingExecutor() : order_id_counter_(0) {
}

bool TradingExecutor::Init(const std::string& config) {
    // 这里可以添加初始化逻辑，比如从配置文件中读取价格追随偏移量、下单数量等
    std::cout << "TradingExecutor initialized with config: " << config << std::endl;
    return true;
}

void TradingExecutor::ProcessLargeOrderSignal(const LargeOrderSignal& signal) {
    // 处理大单信号，执行本方价追随策略
    std::cout << "Processing large order signal for symbol: " << signal.symbol << std::endl;
    ExecuteSameSidePriceFollowingStrategy(signal);
}

std::vector<Order> TradingExecutor::GetAllOrders() const {
    return orders_;
}

Order TradingExecutor::GetOrderById(const std::string& order_id) const {
    for (const auto& order : orders_) {
        if (order.order_id == order_id) {
            return order;
        }
    }

    // 如果订单不存在，返回空订单
    Order empty_order;
    empty_order.timestamp = 0;
    empty_order.symbol = "";
    empty_order.order_type = ' ';
    empty_order.price = 0.0;
    empty_order.volume = 0;
    empty_order.order_id = "";
    empty_order.status = ' ';
    return empty_order;
}

bool TradingExecutor::CancelOrder(const std::string& order_id) {
    for (auto& order : orders_) {
        if (order.order_id == order_id && order.status == 'P') {
            order.status = 'C';
            std::cout << "Order canceled: " << order_id << std::endl;
            return true;
        }
    }

    std::cout << "Failed to cancel order: " << order_id << std::endl;
    return false;
}

void TradingExecutor::Clear() {
    // 清空所有订单
    orders_.clear();
    order_id_counter_ = 0;
}

std::string TradingExecutor::GenerateOrderId() {
    // 生成唯一的订单ID
    std::ostringstream oss;
    oss << "ORD" << std::setw(10) << std::setfill('0') << ++order_id_counter_;
    return oss.str();
}

void TradingExecutor::ExecuteSameSidePriceFollowingStrategy(const LargeOrderSignal& signal) {
    // 执行本方价追随策略
    // 本方价追随策略：当出现大单买入时，以比当前买一价高一个最小变动价位的价格挂买单；
    // 当出现大单卖出时，以比当前卖一价低一个最小变动价位的价格挂卖单。

    // 这里简化处理，直接以大单成交价格为基准，根据方向挂单
    Order order;
    order.timestamp = signal.timestamp;
    order.symbol = signal.symbol;
    order.order_type = signal.direction;
    order.volume = ORDER_VOLUME;
    order.order_id = GenerateOrderId();
    order.status = 'P';

    if (signal.direction == 'B') {
        // 大单买入，挂买单，价格比大单成交价格高一个偏移量
        order.price = signal.price + PRICE_FOLLOWING_OFFSET;
    } else if (signal.direction == 'S') {
        // 大单卖出，挂卖单，价格比大单成交价格低一个偏移量
        order.price = signal.price - PRICE_FOLLOWING_OFFSET;
    }

    // 添加订单到订单列表
    orders_.push_back(order);

    // 打印订单信息
    std::cout << "Order placed: " << std::endl;
    std::cout << "  Timestamp: " << order.timestamp << std::endl;
    std::cout << "  Symbol: " << order.symbol << std::endl;
    std::cout << "  Order type: " << order.order_type << std::endl;
    std::cout << "  Price: " << order.price << std::endl;
    std::cout << "  Volume: " << order.volume << std::endl;
    std::cout << "  Order ID: " << order.order_id << std::endl;
    std::cout << "  Status: " << order.status << std::endl;
}

} // namespace TradingSystem
