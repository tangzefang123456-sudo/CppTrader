#include "trader/signal_generator/signal_generator.h"
#include "trader/matching/order_book.h"
#include <iostream>

namespace CppTrader {
namespace SignalGenerator {

SignalGenerator::SignalGenerator()
    : _large_order_threshold(100000.0) // 默认阈值10万
{
}

SignalGenerator::~SignalGenerator()
{
    Clear();
}

void SignalGenerator::OnTrade(const MarketData::TradeData& trade)
{
    // 计算成交额
    double amount = trade.price * trade.volume;

    // 检查是否触发大单信号
    if (amount >= _large_order_threshold) {
        LargeOrderSignal signal;
        signal.timestamp = trade.timestamp;
        signal.symbol = trade.symbol;
        signal.price = trade.price;
        signal.volume = trade.volume;
        signal.amount = amount;
        signal.direction = trade.direction;

        // 触发大单信号回调
        if (_large_order_signal_callback) {
            _large_order_signal_callback(signal);
        }
    }

    // 更新订单簿
    Matching::OrderBook& order_book = GetOrCreateOrderBook(trade.symbol);
    // 实际实现中应该根据成交数据更新订单簿
    // 这里简化处理，只添加成交记录
}

void SignalGenerator::OnOrder(const MarketData::OrderData& order)
{
    // 更新订单簿
    Matching::OrderBook& order_book = GetOrCreateOrderBook(order.symbol);

    switch (order.status) {
        case 'P': // 已挂单
            if (order.type == 'B') {
                order_book.AddOrder(Matching::OrderType::BUY, order.price, order.volume, order.order_id);
            }
            else if (order.type == 'S') {
                order_book.AddOrder(Matching::OrderType::SELL, order.price, order.volume, order.order_id);
            }
            break;
        case 'C': // 已成交
            order_book.CancelOrder(order.order_id);
            break;
        case 'X': // 已撤销
            order_book.CancelOrder(order.order_id);
            break;
        default:
            break;
    }
}

const Matching::OrderBook* SignalGenerator::GetOrderBook(const std::string& symbol) const
{
    auto it = _order_books.find(symbol);
    if (it != _order_books.end()) {
        return it->second.get();
    }
    return nullptr;
}

void SignalGenerator::Clear()
{
    _order_books.clear();
}

Matching::OrderBook& SignalGenerator::GetOrCreateOrderBook(const std::string& symbol)
{
    auto it = _order_books.find(symbol);
    if (it != _order_books.end()) {
        return *it->second;
    }

    // 创建新的订单簿
    auto order_book = std::make_unique<Matching::OrderBook>();
    Matching::OrderBook& ref = *order_book;
    _order_books[symbol] = std::move(order_book);
    return ref;
}

} // namespace SignalGenerator
} // namespace CppTrader
