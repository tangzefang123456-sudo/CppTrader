/*!
    \file test_signal_generator.cpp
    rief Signal generator module tests
    \author Ivan Shynkarenka
    \date 2024.XX.XX
    \copyright MIT License
*/

#include "trader/signal/signal_generator.h"
#include "trader/matching/market_manager.h"
#include "trader/matching/symbol.h"
#include "trader/matching/order.h"
#include "trader/matching/level.h"

#include <gtest/gtest.h>
#include <string>

using namespace CppTrader;
using namespace CppTrader::Signal;
using namespace CppTrader::Matching;

namespace Test {

class TestSignalHandler : public SignalHandler
{
public:
    bool start_called{false};
    bool stop_called{false};
    int symbol_added_count{0};
    int symbol_removed_count{0};
    int order_book_updated_count{0};
    int trade_count{0};
    int big_trade_signal_count{0};
    int order_count{0};
    int execution_count{0};
    bool error_called{false};

    void onSignalGeneratorStart() override { start_called = true; }
    void onSignalGeneratorStop() override { stop_called = true; }
    void onSymbolAdded(const Symbol& symbol) override { ++symbol_added_count; }
    void onSymbolRemoved(const Symbol& symbol) override { ++symbol_removed_count; }
    void onOrderBookUpdated(const Symbol& symbol, const OrderBook& order_book) override { ++order_book_updated_count; }
    void onTrade(const Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp) override { ++trade_count; }
    void onBigTradeSignal(const Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp, uint64_t amount) override { ++big_trade_signal_count; }
    void onOrder(const Symbol& symbol, const Order& order) override { ++order_count; }
    void onExecution(const Symbol& symbol, const Order& order, uint64_t price, uint64_t quantity, uint64_t timestamp) override { ++execution_count; }
    void onError(const std::string& message) override { error_called = true; }

    void Reset()
    {
        start_called = false;
        stop_called = false;
        symbol_added_count = 0;
        symbol_removed_count = 0;
        order_book_updated_count = 0;
        trade_count = 0;
        big_trade_signal_count = 0;
        order_count = 0;
        execution_count = 0;
        error_called = false;
    }
};

TEST(Signal, SignalGenerator) {
    // Create a market manager
    MarketManager market_manager;

    // Create a signal generator
    SignalGenerator signal_generator(market_manager);
    EXPECT_FALSE(signal_generator.IsRunning());

    // Create a test handler
    TestSignalHandler handler;
    signal_generator.SetHandler(&handler);

    // Start the signal generator
    EXPECT_TRUE(signal_generator.Start());
    EXPECT_TRUE(signal_generator.IsRunning());
    EXPECT_TRUE(handler.start_called);

    // Create a test symbol
    Symbol symbol(1, "AAPL");

    // Add the symbol to the signal generator
    signal_generator.onMarketDataSymbol(symbol);
    EXPECT_EQ(handler.symbol_added_count, 1);

    // Create a test order book update
    Level level(LevelType::BID, 1000000, 100, 0, 100, 1);
    LevelUpdate update(UpdateType::ADD, level, true);

    // Update the order book
    signal_generator.onMarketDataOrderBookUpdate(symbol, update);
    EXPECT_EQ(handler.order_book_updated_count, 1);

    // Create a test trade (small trade - should not generate a signal)
    uint64_t timestamp = CppCommon::Timestamp::Now().timestamp();
    signal_generator.onMarketDataTrade(symbol, 1000000, 100, timestamp); // Amount = 100,000,000
    EXPECT_EQ(handler.trade_count, 1);
    EXPECT_EQ(handler.big_trade_signal_count, 0);

    // Create a test trade (big trade - should generate a signal)
    signal_generator.onMarketDataTrade(symbol, 1000000, 101, timestamp); // Amount = 101,000,000
    EXPECT_EQ(handler.trade_count, 2);
    EXPECT_EQ(handler.big_trade_signal_count, 1);

    // Create a test order
    Order order = Order::Limit(1, 1, OrderSide::BUY, 1000000, 100);
    signal_generator.onMarketDataOrder(symbol, order);
    EXPECT_EQ(handler.order_count, 1);

    // Create a test execution
    signal_generator.onMarketDataExecution(symbol, order, 1000000, 100, timestamp);
    EXPECT_EQ(handler.execution_count, 1);

    // Test error handling
    signal_generator.onMarketDataError("Test error");
    EXPECT_TRUE(handler.error_called);

    // Stop the signal generator
    EXPECT_TRUE(signal_generator.Stop());
    EXPECT_FALSE(signal_generator.IsRunning());
    EXPECT_TRUE(handler.stop_called);
}

TEST(Signal, SignalGeneratorBigTradeThreshold) {
    // Create a market manager
    MarketManager market_manager;

    // Create a signal generator with a custom big trade threshold
    SignalGenerator signal_generator(market_manager);
    signal_generator.SetBigTradeThreshold(200000); // 200,000
    EXPECT_EQ(signal_generator.big_trade_threshold(), 200000);

    // Create a test handler
    TestSignalHandler handler;
    signal_generator.SetHandler(&handler);

    // Start the signal generator
    EXPECT_TRUE(signal_generator.Start());

    // Create a test symbol
    Symbol symbol(1, "AAPL");
    signal_generator.onMarketDataSymbol(symbol);

    // Create a test trade (below threshold - should not generate a signal)
    uint64_t timestamp = CppCommon::Timestamp::Now().timestamp();
    signal_generator.onMarketDataTrade(symbol, 1000000, 150, timestamp); // Amount = 150,000,000
    EXPECT_EQ(handler.trade_count, 1);
    EXPECT_EQ(handler.big_trade_signal_count, 0);

    // Create a test trade (above threshold - should generate a signal)
    signal_generator.onMarketDataTrade(symbol, 1000000, 250, timestamp); // Amount = 250,000,000
    EXPECT_EQ(handler.trade_count, 2);
    EXPECT_EQ(handler.big_trade_signal_count, 1);

    // Stop the signal generator
    EXPECT_TRUE(signal_generator.Stop());
}

TEST(Signal, SignalGeneratorRestart) {
    // Create a market manager
    MarketManager market_manager;

    // Create a signal generator
    SignalGenerator signal_generator(market_manager);

    // Create a test handler
    TestSignalHandler handler;
    signal_generator.SetHandler(&handler);

    // Start the signal generator
    EXPECT_TRUE(signal_generator.Start());
    EXPECT_TRUE(signal_generator.IsRunning());
    EXPECT_TRUE(handler.start_called);

    // Restart the signal generator
    handler.Reset();
    EXPECT_TRUE(signal_generator.Restart());
    EXPECT_TRUE(signal_generator.IsRunning());
    EXPECT_TRUE(handler.start_called);
    EXPECT_FALSE(handler.stop_called);

    // Stop the signal generator
    EXPECT_TRUE(signal_generator.Stop());
}

} // namespace Test
