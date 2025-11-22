/*!
    \file test_execution_manager.cpp
    \brief Execution manager module tests
    \author Ivan Shynkarenka
    \date 2024.XX.XX
    \copyright MIT License
*/

#include "trader/execution/execution_manager.h"
#include "trader/matching/market_manager.h"
#include "trader/matching/symbol.h"
#include "trader/matching/order.h"
#include "trader/matching/level.h"
#include "trader/matching/trade.h"

#include <gtest/gtest.h>
#include <string>

using namespace CppTrader;
using namespace CppTrader::Execution;
using namespace CppTrader::Matching;
using namespace CppTrader::Signal;

namespace Test {

class TestMarketManager : public MarketManager
{
public:
    bool order_added{false};
    bool order_canceled{false};
    bool order_replaced{false};
    Order last_order;

    bool AddOrder(const Order& order) override
    {
        order_added = true;
        last_order = order;
        return true;
    }

    bool CancelOrder(uint64_t id) override
    {
        order_canceled = true;
        return true;
    }

    bool ReplaceOrder(uint64_t id, uint64_t price, uint64_t quantity) override
    {
        order_replaced = true;
        return true;
    }

    void Reset()
    {
        order_added = false;
        order_canceled = false;
        order_replaced = false;
        last_order = Order();
    }
};

class TestExecutionHandler : public ExecutionHandler
{
public:
    bool start_called{false};
    bool stop_called{false};
    int symbol_added_count{0};
    int symbol_removed_count{0};
    int order_book_updated_count{0};
    int trade_count{0};
    int order_count{0};
    int execution_count{0};
    int order_added_count{0};
    int order_canceled_count{0};
    int order_replaced_count{0};
    bool error_called{false};

    void onExecutionManagerStart() override { start_called = true; }
    void onExecutionManagerStop() override { stop_called = true; }
    void onSymbolAdded(const Symbol& symbol) override { ++symbol_added_count; }
    void onSymbolRemoved(const Symbol& symbol) override { ++symbol_removed_count; }
    void onOrderBookUpdated(const Symbol& symbol, const OrderBook& order_book) override { ++order_book_updated_count; }
    void onTrade(const Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp) override { ++trade_count; }
    void onOrder(const Symbol& symbol, const Order& order) override { ++order_count; }
    void onExecution(const Symbol& symbol, const Order& order, uint64_t price, uint64_t quantity, uint64_t timestamp) override { ++execution_count; }
    void onOrderAdded(const Symbol& symbol, const Order& order) override { ++order_added_count; }
    void onOrderCanceled(const Symbol& symbol, const Order& order) override { ++order_canceled_count; }
    void onOrderReplaced(const Symbol& symbol, const Order& order) override { ++order_replaced_count; }
    void onError(const std::string& message) override { error_called = true; }

    void Reset()
    {
        start_called = false;
        stop_called = false;
        symbol_added_count = 0;
        symbol_removed_count = 0;
        order_book_updated_count = 0;
        trade_count = 0;
        order_count = 0;
        execution_count = 0;
        order_added_count = 0;
        order_canceled_count = 0;
        order_replaced_count = 0;
        error_called = false;
    }
};

TEST(Execution, ExecutionManager) {
    // Create a test market manager
    TestMarketManager market_manager;

    // Create an execution manager
    ExecutionManager execution_manager(market_manager);
    EXPECT_FALSE(execution_manager.IsRunning());

    // Create a test handler
    TestExecutionHandler handler;
    execution_manager.SetHandler(&handler);

    // Start the execution manager
    EXPECT_TRUE(execution_manager.Start());
    EXPECT_TRUE(execution_manager.IsRunning());
    EXPECT_TRUE(handler.start_called);

    // Create a test symbol
    Symbol symbol(1, "AAPL");

    // Add the symbol to the execution manager
    execution_manager.onMarketDataSymbol(symbol);
    EXPECT_EQ(handler.symbol_added_count, 1);

    // Create a test order book update with bid and ask levels
    Level bid_level(LevelType::BID, 1000000, 100, 0, 100, 1);
    Level ask_level(LevelType::ASK, 1000100, 100, 0, 100, 1);
    LevelUpdate bid_update(UpdateType::ADD, bid_level, true);
    LevelUpdate ask_update(UpdateType::ADD, ask_level, true);

    // Update the order book
    execution_manager.onMarketDataOrderBookUpdate(symbol, bid_update);
    execution_manager.onMarketDataOrderBookUpdate(symbol, ask_update);
    EXPECT_EQ(handler.order_book_updated_count, 2);

    // Test big trade signal (should execute follow strategy)
    uint64_t timestamp = CppCommon::Timestamp::Now().timestamp();
    execution_manager.onBigTradeSignal(symbol, 1000000, 101, timestamp, 101000000); // Amount = 101,000,000
    EXPECT_EQ(handler.order_added_count, 1);
    EXPECT_TRUE(market_manager.order_added);
    EXPECT_EQ(market_manager.last_order.side(), OrderSide::BUY);
    EXPECT_EQ(market_manager.last_order.price(), 1000000); // Same as best bid
    EXPECT_EQ(market_manager.last_order.quantity(), 101); // Same as trade quantity

    // Test big trade signal on ask side
    market_manager.Reset();
    handler.Reset();
    execution_manager.onBigTradeSignal(symbol, 1000100, 150, timestamp, 150015000); // Amount = 150,015,000
    EXPECT_EQ(handler.order_added_count, 1);
    EXPECT_TRUE(market_manager.order_added);
    EXPECT_EQ(market_manager.last_order.side(), OrderSide::SELL);
    EXPECT_EQ(market_manager.last_order.price(), 1000100); // Same as best ask
    EXPECT_EQ(market_manager.last_order.quantity(), 150); // Same as trade quantity

    // Test order handling
    Order order = Order::Limit(1, 1, OrderSide::BUY, 1000000, 100);
    execution_manager.onMarketDataOrder(symbol, order);
    EXPECT_EQ(handler.order_count, 1);

    // Test execution handling
    execution_manager.onMarketDataExecution(symbol, order, 1000000, 100, timestamp);
    EXPECT_EQ(handler.execution_count, 1);

    // Test error handling
    execution_manager.onMarketDataError("Test error");
    EXPECT_TRUE(handler.error_called);

    // Stop the execution manager
    EXPECT_TRUE(execution_manager.Stop());
    EXPECT_FALSE(execution_manager.IsRunning());
    EXPECT_TRUE(handler.stop_called);
}

TEST(Execution, ExecutionManagerFollowStrategy) {
    // Create a test market manager
    TestMarketManager market_manager;

    // Create an execution manager with custom follow strategy parameters
    ExecutionManager execution_manager(market_manager);
    execution_manager.SetFollowStrategyParameters(5, 10000); // Max orders: 5, Max quantity: 10,000
    EXPECT_EQ(execution_manager.follow_max_orders(), 5);
    EXPECT_EQ(execution_manager.follow_max_quantity(), 10000);

    // Create a test handler
    TestExecutionHandler handler;
    execution_manager.SetHandler(&handler);

    // Start the execution manager
    EXPECT_TRUE(execution_manager.Start());

    // Create a test symbol
    Symbol symbol(1, "AAPL");
    execution_manager.onMarketDataSymbol(symbol);

    // Create a test order book update with bid and ask levels
    Level bid_level(LevelType::BID, 1000000, 100, 0, 100, 1);
    Level ask_level(LevelType::ASK, 1000100, 100, 0, 100, 1);
    LevelUpdate bid_update(UpdateType::ADD, bid_level, true);
    LevelUpdate ask_update(UpdateType::ADD, ask_level, true);
    execution_manager.onMarketDataOrderBookUpdate(symbol, bid_update);
    execution_manager.onMarketDataOrderBookUpdate(symbol, ask_update);

    // Test big trade signal with quantity below max
    uint64_t timestamp = CppCommon::Timestamp::Now().timestamp();
    execution_manager.onBigTradeSignal(symbol, 1000000, 5000, timestamp, 5000000000); // Amount = 5,000,000,000
    EXPECT_EQ(handler.order_added_count, 1);
    EXPECT_TRUE(market_manager.order_added);
    EXPECT_EQ(market_manager.last_order.quantity(), 5000); // Same as trade quantity

    // Test big trade signal with quantity above max
    market_manager.Reset();
    handler.Reset();
    execution_manager.onBigTradeSignal(symbol, 1000000, 15000, timestamp, 15000000000); // Amount = 15,000,000,000
    EXPECT_EQ(handler.order_added_count, 1);
    EXPECT_TRUE(market_manager.order_added);
    EXPECT_EQ(market_manager.last_order.quantity(), 10000); // Limited to max quantity

    // Stop the execution manager
    EXPECT_TRUE(execution_manager.Stop());
}

TEST(Execution, ExecutionManagerRestart) {
    // Create a test market manager
    TestMarketManager market_manager;

    // Create an execution manager
    ExecutionManager execution_manager(market_manager);

    // Create a test handler
    TestExecutionHandler handler;
    execution_manager.SetHandler(&handler);

    // Start the execution manager
    EXPECT_TRUE(execution_manager.Start());
    EXPECT_TRUE(execution_manager.IsRunning());
    EXPECT_TRUE(handler.start_called);

    // Restart the execution manager
    handler.Reset();
    EXPECT_TRUE(execution_manager.Restart());
    EXPECT_TRUE(execution_manager.IsRunning());
    EXPECT_TRUE(handler.start_called);
    EXPECT_FALSE(handler.stop_called);

    // Stop the execution manager
    EXPECT_TRUE(execution_manager.Stop());
}

} // namespace Test
