#include "test.h"
#include "trader/signal_generator/signal_generator.h"

using namespace CppTrader;

TEST_CASE("SignalGenerator", "[SignalGenerator]")
{
    // 创建信号产生器
    SignalGenerator::SignalGenerator signal_generator;

    // 测试大单阈值设置
    REQUIRE(signal_generator.GetLargeOrderThreshold() == 100000.0);
    signal_generator.SetLargeOrderThreshold(50000.0);
    REQUIRE(signal_generator.GetLargeOrderThreshold() == 50000.0);

    // 测试大单信号触发
    bool signal_triggered = false;
    SignalGenerator::LargeOrderSignal received_signal;

    signal_generator.SetLargeOrderSignalCallback([&](const SignalGenerator::LargeOrderSignal& signal) {
        signal_triggered = true;
        received_signal = signal;
    });

    // 生成一个小单成交
    MarketData::TradeData small_trade;
    small_trade.timestamp = 1234567890;
    small_trade.symbol = "AAPL";
    small_trade.price = 150.0;
    small_trade.volume = 100; // 成交额15000，小于阈值50000
    small_trade.direction = 'B';

    signal_triggered = false;
    signal_generator.OnTrade(small_trade);
    REQUIRE(signal_triggered == false);

    // 生成一个大单成交
    MarketData::TradeData large_trade;
    large_trade.timestamp = 1234567891;
    large_trade.symbol = "AAPL";
    large_trade.price = 150.0;
    large_trade.volume = 500; // 成交额75000，大于阈值50000
    large_trade.direction = 'S';

    signal_triggered = false;
    signal_generator.OnTrade(large_trade);
    REQUIRE(signal_triggered == true);
    REQUIRE(received_signal.timestamp == large_trade.timestamp);
    REQUIRE(received_signal.symbol == large_trade.symbol);
    REQUIRE(received_signal.price == large_trade.price);
    REQUIRE(received_signal.volume == large_trade.volume);
    REQUIRE(received_signal.amount == large_trade.price * large_trade.volume);
    REQUIRE(received_signal.direction == large_trade.direction);

    // 测试订单簿管理
    const Matching::OrderBook* order_book = signal_generator.GetOrderBook("AAPL");
    REQUIRE(order_book != nullptr);

    order_book = signal_generator.GetOrderBook("GOOG");
    REQUIRE(order_book == nullptr);

    // 测试清除订单簿
    signal_generator.Clear();
    order_book = signal_generator.GetOrderBook("AAPL");
    REQUIRE(order_book == nullptr);
}
