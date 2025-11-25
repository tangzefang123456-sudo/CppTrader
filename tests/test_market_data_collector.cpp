#include "test.h"
#include "trader/market_data/market_data_collector.h"

using namespace CppTrader;

TEST_CASE("MarketDataRecorder", "[MarketData]")
{
    // 创建行情刻录器
    MarketData::MarketDataRecorder recorder("test_market_data.bin");

    // 测试打开和关闭文件
    REQUIRE(recorder.Open() == true);
    REQUIRE(recorder.IsOpen() == true);
    REQUIRE(recorder.Close() == true);
    REQUIRE(recorder.IsOpen() == false);

    // 测试重复打开和关闭
    REQUIRE(recorder.Open() == true);
    REQUIRE(recorder.Open() == true); // 已经打开，应该返回true
    REQUIRE(recorder.Close() == true);
    REQUIRE(recorder.Close() == true); // 已经关闭，应该返回true

    // 测试刻录数据
    REQUIRE(recorder.Open() == true);

    MarketData::TradeData trade;
    trade.timestamp = 1234567890;
    trade.symbol = "AAPL";
    trade.price = 150.0;
    trade.volume = 100;
    trade.direction = 'B';

    MarketData::OrderData order;
    order.timestamp = 1234567891;
    order.symbol = "AAPL";
    order.order_id = 10001;
    order.type = 'B';
    order.price = 150.0;
    order.volume = 100;
    order.status = 'P';

    REQUIRE(recorder.RecordTrade(trade) == true);
    REQUIRE(recorder.RecordOrder(order) == true);

    REQUIRE(recorder.Close() == true);
}

TEST_CASE("MarketDataPlayer", "[MarketData]")
{
    // 创建行情播放器
    MarketData::MarketDataPlayer player("test_market_data.bin");

    // 测试打开和关闭文件
    REQUIRE(player.Open() == true);
    REQUIRE(player.IsOpen() == true);
    REQUIRE(player.Close() == true);
    REQUIRE(player.IsOpen() == false);

    // 测试重复打开和关闭
    REQUIRE(player.Open() == true);
    REQUIRE(player.Open() == true); // 已经打开，应该返回true
    REQUIRE(player.Close() == true);
    REQUIRE(player.Close() == true); // 已经关闭，应该返回true

    // 测试播放控制
    REQUIRE(player.Open() == true);
    REQUIRE(player.Play(1.0) == true);
    REQUIRE(player.Pause() == true);
    REQUIRE(player.Resume() == true);
    REQUIRE(player.Stop() == true);
    REQUIRE(player.Close() == true);
}
