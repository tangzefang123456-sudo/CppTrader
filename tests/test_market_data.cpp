/*!
    \file test_market_data.cpp
    rief Market data module tests
    \author Ivan Shynkarenka
    \date 2024.XX.XX
    \copyright MIT License
*/

#include "trader/market_data/market_data_handler.h"
#include "trader/market_data/market_data_recorder.h"
#include "trader/market_data/market_data_player.h"
#include "trader/matching/symbol.h"
#include "trader/matching/order.h"
#include "trader/matching/level.h"

#include <gtest/gtest.h>
#include <fstream>
#include <string>

using namespace CppTrader;
using namespace CppTrader::MarketData;
using namespace CppTrader::Matching;

namespace Test {

class TestMarketDataHandler : public MarketDataHandler::Handler
{
public:
    bool start_called{false};
    bool stop_called{false};
    int symbol_count{0};
    int order_book_update_count{0};
    int trade_count{0};
    int order_count{0};
    int execution_count{0};
    bool error_called{false};

    void onMarketDataStart() override { start_called = true; }
    void onMarketDataStop() override { stop_called = true; }
    void onMarketDataSymbol(const Symbol& symbol) override { ++symbol_count; }
    void onMarketDataOrderBookUpdate(const Symbol& symbol, const LevelUpdate& update) override { ++order_book_update_count; }
    void onMarketDataTrade(const Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp) override { ++trade_count; }
    void onMarketDataOrder(const Symbol& symbol, const Order& order) override { ++order_count; }
    void onMarketDataExecution(const Symbol& symbol, const Order& order, uint64_t price, uint64_t quantity, uint64_t timestamp) override { ++execution_count; }
    void onMarketDataError(const std::string& message) override { error_called = true; }

    void Reset()
    {
        start_called = false;
        stop_called = false;
        symbol_count = 0;
        order_book_update_count = 0;
        trade_count = 0;
        order_count = 0;
        execution_count = 0;
        error_called = false;
    }
};

TEST(MarketData, MarketDataRecorder) {
    // Create a temporary filename
    std::string filename = "test_market_data.bin";

    // Create a market data recorder
    MarketDataRecorder recorder(filename);
    EXPECT_TRUE(recorder.IsOpened());

    // Create test data
    Symbol symbol(1, "AAPL");
    Order order = Order::Limit(1, 1, OrderSide::BUY, 1000000, 100);
    Level level(LevelType::BID, 1000000, 100, 0, 100, 1);
    LevelUpdate update(UpdateType::ADD, level, true);
    uint64_t timestamp = CppCommon::Timestamp::Now().timestamp();

    // Write test data to the recorder
    recorder.WriteStart();
    recorder.WriteSymbol(symbol);
    recorder.WriteOrderBookUpdate(symbol, update);
    recorder.WriteTrade(symbol, 1000000, 100, timestamp);
    recorder.WriteOrder(symbol, order);
    recorder.WriteExecution(symbol, order, 1000000, 100, timestamp);
    recorder.WriteError("Test error");
    recorder.WriteStop();

    // Close the recorder
    EXPECT_TRUE(recorder.Close());
    EXPECT_FALSE(recorder.IsOpened());

    // Check if the file was created and has content
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    EXPECT_TRUE(file.is_open());
    EXPECT_GT(file.tellg(), 0);
    file.close();

    // Delete the temporary file
    std::remove(filename.c_str());
}

TEST(MarketData, MarketDataPlayer) {
    // Create a temporary filename
    std::string filename = "test_market_data.bin";

    // Create test data and write it to a file
    {
        MarketDataRecorder recorder(filename);
        EXPECT_TRUE(recorder.IsOpened());

        Symbol symbol(1, "AAPL");
        Order order = Order::Limit(1, 1, OrderSide::BUY, 1000000, 100);
        Level level(LevelType::BID, 1000000, 100, 0, 100, 1);
        LevelUpdate update(UpdateType::ADD, level, true);
        uint64_t timestamp = CppCommon::Timestamp::Now().timestamp();

        recorder.WriteStart();
        recorder.WriteSymbol(symbol);
        recorder.WriteOrderBookUpdate(symbol, update);
        recorder.WriteTrade(symbol, 1000000, 100, timestamp);
        recorder.WriteOrder(symbol, order);
        recorder.WriteExecution(symbol, order, 1000000, 100, timestamp);
        recorder.WriteError("Test error");
        recorder.WriteStop();

        EXPECT_TRUE(recorder.Close());
    }

    // Create a market data player
    MarketDataPlayer player(filename);
    EXPECT_TRUE(player.IsOpened());

    // Create a test handler
    TestMarketDataHandler handler;
    player.SetHandler(&handler);

    // Play the market data
    EXPECT_TRUE(player.Play());
    EXPECT_TRUE(player.IsPlaying());

    // Wait for the player to finish
    while (player.IsPlaying())
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Check if all events were handled
    EXPECT_TRUE(handler.start_called);
    EXPECT_TRUE(handler.stop_called);
    EXPECT_EQ(handler.symbol_count, 1);
    EXPECT_EQ(handler.order_book_update_count, 1);
    EXPECT_EQ(handler.trade_count, 1);
    EXPECT_EQ(handler.order_count, 1);
    EXPECT_EQ(handler.execution_count, 1);
    EXPECT_TRUE(handler.error_called);

    // Close the player
    EXPECT_TRUE(player.Close());
    EXPECT_FALSE(player.IsOpened());

    // Delete the temporary file
    std::remove(filename.c_str());
}

TEST(MarketData, MarketDataPlayerSpeed) {
    // Create a temporary filename
    std::string filename = "test_market_data_speed.bin";

    // Create test data with multiple trade messages
    {
        MarketDataRecorder recorder(filename);
        EXPECT_TRUE(recorder.IsOpened());

        Symbol symbol(1, "AAPL");
        uint64_t timestamp = CppCommon::Timestamp::Now().timestamp();

        recorder.WriteStart();
        recorder.WriteSymbol(symbol);

        // Write 100 trade messages
        for (int i = 0; i < 100; ++i)
        {
            recorder.WriteTrade(symbol, 1000000 + i, 100, timestamp + i * 1000000); // 1 second apart
        }

        recorder.WriteStop();

        EXPECT_TRUE(recorder.Close());
    }

    // Create a market data player
    MarketDataPlayer player(filename);
    EXPECT_TRUE(player.IsOpened());

    // Create a test handler
    TestMarketDataHandler handler;
    player.SetHandler(&handler);

    // Set double speed
    player.SetSpeed(2.0);
    EXPECT_EQ(player.speed(), 2.0);

    // Play the market data
    EXPECT_TRUE(player.Play());

    // Wait for the player to finish
    while (player.IsPlaying())
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Check if all trade events were handled
    EXPECT_EQ(handler.trade_count, 100);

    // Close the player
    EXPECT_TRUE(player.Close());

    // Delete the temporary file
    std::remove(filename.c_str());
}

} // namespace Test
