#include "market_data_processor.h"
#include <iostream>

namespace TradingSystem {

MarketDataProcessor::MarketDataProcessor() {
    // 初始化买单队列（价格从高到低）
    auto bid_compare = [](const OrderBookEntry& a, const OrderBookEntry& b) {
        return a.price < b.price;
    };

    // 初始化卖单队列（价格从低到高）
    auto ask_compare = [](const OrderBookEntry& a, const OrderBookEntry& b) {
        return a.price > b.price;
    };

    // 为每个标的初始化订单簿
    // 这里暂时不添加任何标的，等到处理数据时再动态添加
}

bool MarketDataProcessor::Init(const std::string& config) {
    // 这里可以添加初始化逻辑，比如从配置文件中读取大单阈值等
    std::cout << "MarketDataProcessor initialized with config: " << config << std::endl;
    return true;
}

void MarketDataProcessor::ProcessTickData(const TickData& tick) {
    // 处理逐笔成交数据，更新订单簿
    auto it = order_books_.find(tick.symbol);
    if (it == order_books_.end()) {
        // 如果标的不存在，创建新的订单簿
        auto bid_compare = [](const OrderBookEntry& a, const OrderBookEntry& b) {
            return a.price < b.price;
        };

        auto ask_compare = [](const OrderBookEntry& a, const OrderBookEntry& b) {
            return a.price > b.price;
        };

        OrderBook order_book;
        order_book.bids = std::priority_queue<OrderBookEntry, std::vector<OrderBookEntry>, 
                                             decltype(bid_compare)>(bid_compare);
        order_book.asks = std::priority_queue<OrderBookEntry, std::vector<OrderBookEntry>, 
                                             decltype(ask_compare)>(ask_compare);

        order_books_[tick.symbol] = order_book;
        it = order_books_.find(tick.symbol);
    }

    // 根据成交方向更新订单簿
    if (tick.direction == 'B') {
        // 买入成交，减少卖单队列中的数量
        if (!it->second.asks.empty()) {
            OrderBookEntry top_ask = it->second.asks.top();
            it->second.asks.pop();

            top_ask.volume -= tick.volume;
            if (top_ask.volume > 0) {
                it->second.asks.push(top_ask);
            }
        }
    } else if (tick.direction == 'S') {
        // 卖出成交，减少买单队列中的数量
        if (!it->second.bids.empty()) {
            OrderBookEntry top_bid = it->second.bids.top();
            it->second.bids.pop();

            top_bid.volume -= tick.volume;
            if (top_bid.volume > 0) {
                it->second.bids.push(top_bid);
            }
        }
    }

    // 检查是否需要生成大单信号
    double amount = tick.price * tick.volume;
    if (amount > LARGE_ORDER_THRESHOLD) {
        GenerateLargeOrderSignal(tick);
    }
}

void MarketDataProcessor::ProcessOrderData(const OrderData& order) {
    // 处理逐笔委托数据，更新订单簿
    auto it = order_books_.find(order.symbol);
    if (it == order_books_.end()) {
        // 如果标的不存在，创建新的订单簿
        auto bid_compare = [](const OrderBookEntry& a, const OrderBookEntry& b) {
            return a.price < b.price;
        };

        auto ask_compare = [](const OrderBookEntry& a, const OrderBookEntry& b) {
            return a.price > b.price;
        };

        OrderBook order_book;
        order_book.bids = std::priority_queue<OrderBookEntry, std::vector<OrderBookEntry>, 
                                             decltype(bid_compare)>(bid_compare);
        order_book.asks = std::priority_queue<OrderBookEntry, std::vector<OrderBookEntry>, 
                                             decltype(ask_compare)>(ask_compare);

        order_books_[order.symbol] = order_book;
        it = order_books_.find(order.symbol);
    }

    // 根据委托类型更新订单簿
    if (order.order_type == 'B') {
        // 买入委托，添加到买单队列
        OrderBookEntry entry;
        entry.price = order.price;
        entry.volume = order.volume;
        it->second.bids.push(entry);
    } else if (order.order_type == 'S') {
        // 卖出委托，添加到卖单队列
        OrderBookEntry entry;
        entry.price = order.price;
        entry.volume = order.volume;
        it->second.asks.push(entry);
    }
}

OrderBook MarketDataProcessor::GetOrderBook(const std::string& symbol) const {
    auto it = order_books_.find(symbol);
    if (it != order_books_.end()) {
        return it->second;
    } else {
        // 如果标的不存在，返回空的订单簿
        auto bid_compare = [](const OrderBookEntry& a, const OrderBookEntry& b) {
            return a.price < b.price;
        };

        auto ask_compare = [](const OrderBookEntry& a, const OrderBookEntry& b) {
            return a.price > b.price;
        };

        OrderBook empty_order_book;
        empty_order_book.bids = std::priority_queue<OrderBookEntry, std::vector<OrderBookEntry>, 
                                                   decltype(bid_compare)>(bid_compare);
        empty_order_book.asks = std::priority_queue<OrderBookEntry, std::vector<OrderBookEntry>, 
                                                   decltype(ask_compare)>(ask_compare);
        return empty_order_book;
    }
}

std::vector<LargeOrderSignal> MarketDataProcessor::GetLatestLargeOrderSignals() {
    // 返回最新的大单信号，并清空缓存
    std::vector<LargeOrderSignal> signals = large_order_signals_;
    large_order_signals_.clear();
    return signals;
}

void MarketDataProcessor::Clear() {
    // 清空所有数据
    order_books_.clear();
    large_order_signals_.clear();
}

void MarketDataProcessor::GenerateLargeOrderSignal(const TickData& tick) {
    // 生成大单信号
    LargeOrderSignal signal;
    signal.timestamp = tick.timestamp;
    signal.symbol = tick.symbol;
    signal.price = tick.price;
    signal.volume = tick.volume;
    signal.amount = tick.price * tick.volume;
    signal.direction = tick.direction;

    large_order_signals_.push_back(signal);

    // 打印大单信号信息
    std::cout << "Large order signal generated: " << std::endl;
    std::cout << "  Timestamp: " << signal.timestamp << std::endl;
    std::cout << "  Symbol: " << signal.symbol << std::endl;
    std::cout << "  Price: " << signal.price << std::endl;
    std::cout << "  Volume: " << signal.volume << std::endl;
    std::cout << "  Amount: " << signal.amount << std::endl;
    std::cout << "  Direction: " << signal.direction << std::endl;
}

} // namespace TradingSystem
