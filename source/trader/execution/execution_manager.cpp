#include "trader/execution/execution_manager.h"
#include <iostream>

namespace CppTrader {
namespace Execution {

ExecutionManager::ExecutionManager(std::unique_ptr<ExecutionHandler> execution_handler)
    : _execution_handler(std::move(execution_handler)), _price_offset(0.01), _follow_quantity(100)
{
}

ExecutionManager::~ExecutionManager()
{
}

void ExecutionManager::OnLargeOrderSignal(const SignalGenerator::LargeOrderSignal& signal)
{
    // 执行本方价追随策略
    ExecuteFollowStrategy(signal);
}

void ExecutionManager::OnTrade(const MarketData::TradeData& trade)
{
    // 处理行情数据，更新最新价格等信息
    // 这里简化处理，实际实现中应该维护每个标的的最新行情
}

void ExecutionManager::OnOrder(const MarketData::OrderData& order)
{
    // 处理委托数据，更新订单簿等信息
    // 这里简化处理，实际实现中应该维护每个标的的订单簿
}

void ExecutionManager::ExecuteFollowStrategy(const SignalGenerator::LargeOrderSignal& signal)
{
    if (!_execution_handler) {
        return;
    }

    OrderRequest request;
    request.symbol = signal.symbol;

    // 根据大单方向设置追随方向
    if (signal.direction == 'B') {
        // 大单买入，挂本方买价追随（通常是最新买价+偏移）
        request.side = 'B';
        // 实际实现中应该获取最新的买一价
        request.price = signal.price + _price_offset;
    }
    else if (signal.direction == 'S') {
        // 大单卖出，挂本方卖价追随（通常是最新卖价-偏移）
        request.side = 'S';
        // 实际实现中应该获取最新的卖一价
        request.price = signal.price - _price_offset;
    }
    else {
        // 未知方向，不执行
        return;
    }

    request.quantity = _follow_quantity;

    // 提交委托订单
    uint64_t order_id = _execution_handler->SubmitOrder(request);
    std::cout << "Submitted follow order: ID=" << order_id << ", Symbol=" << request.symbol << ", Side=" << request.side << ", Price=" << request.price << ", Quantity=" << request.quantity << std::endl;
}

} // namespace Execution
} // namespace CppTrader
