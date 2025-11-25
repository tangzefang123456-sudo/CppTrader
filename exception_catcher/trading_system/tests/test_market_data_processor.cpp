#include "gtest/gtest.h"
#include "market_data_processor.h"
#include <string>

using namespace TradingSystem;

class MarketDataProcessorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建行情数据处理器实例
        processor_ = new MarketDataProcessor();
        ASSERT_NE(processor_, nullptr);

        // 初始化行情数据处理器
        bool init_result = processor_->Init("test_config");
        ASSERT_TRUE(init_result);
    }

    void TearDown() override {
        // 清空行情数据处理器
        processor_->Clear();

        // 销毁行情数据处理器实例
        delete processor_;
        processor_ = nullptr;
    }

    MarketDataProcessor* processor_;
};

TEST_F(MarketDataProcessorTest, ProcessTickData) {
    // 创建测试用的逐笔成交数据
    TickData tick;
    tick.timestamp = 1234567890123456789ULL;
    tick.symbol = "000001.SZ";
    tick.price = 10.0;
    tick.volume = 100;
    tick.direction = 'B';

    // 处理逐笔成交数据
    processor_->ProcessTickData(tick);

    // 获取订单簿
    OrderBook order_book = processor_->GetOrderBook("000001.SZ");

    // 验证订单簿是否正确
    // 由于这是一笔买入成交，卖单队列应该减少
    // 但由于之前没有委托数据，订单簿应该是空的
    ASSERT_TRUE(order_book.bids.empty());
    ASSERT_TRUE(order_book.asks.empty());
}

TEST_F(MarketDataProcessorTest, ProcessOrderData) {
    // 创建测试用的逐笔委托数据
    OrderData order;
    order.timestamp = 1234567890123456789ULL;
    order.symbol = "000001.SZ";
    order.price = 10.0;
    order.volume = 100;
    order.order_type = 'B';
    order.order_id = "ORDER000001";

    // 处理逐笔委托数据
    processor_->ProcessOrderData(order);

    // 获取订单簿
    OrderBook order_book = processor_->GetOrderBook("000001.SZ");

    // 验证订单簿是否正确
    // 由于这是一笔买入委托，买单队列应该有一个条目
    ASSERT_FALSE(order_book.bids.empty());
    ASSERT_TRUE(order_book.asks.empty());

    // 验证买单队列中的价格和数量是否正确
    OrderBookEntry top_bid = order_book.bids.top();
    ASSERT_DOUBLE_EQ(top_bid.price, 10.0);
    ASSERT_EQ(top_bid.volume, 100);
}

TEST_F(MarketDataProcessorTest, GenerateLargeOrderSignal) {
    // 创建测试用的大额逐笔成交数据（成交额大于10万）
    TickData large_tick;
    large_tick.timestamp = 1234567890123456789ULL;
    large_tick.symbol = "000001.SZ";
    large_tick.price = 100.0;
    large_tick.volume = 1001;  // 成交额为100.0 * 1001 = 100100.0 > 10万
    large_tick.direction = 'B';

    // 处理大额逐笔成交数据
    processor_->ProcessTickData(large_tick);

    // 获取最新的大单信号
    std::vector<LargeOrderSignal> signals = processor_->GetLatestLargeOrderSignals();

    // 验证是否生成了大单信号
    ASSERT_EQ(signals.size(), 1);

    // 验证大单信号的内容是否正确
    const LargeOrderSignal& signal = signals[0];
    ASSERT_EQ(signal.timestamp, 1234567890123456789ULL);
    ASSERT_EQ(signal.symbol, "000001.SZ");
    ASSERT_DOUBLE_EQ(signal.price, 100.0);
    ASSERT_EQ(signal.volume, 1001);
    ASSERT_DOUBLE_EQ(signal.amount, 100.0 * 1001);
    ASSERT_EQ(signal.direction, 'B');
}

TEST_F(MarketDataProcessorTest, NotGenerateLargeOrderSignal) {
    // 创建测试用的小额逐笔成交数据（成交额小于10万）
    TickData small_tick;
    small_tick.timestamp = 1234567890123456789ULL;
    strcpy_s(small_tick.symbol, "000001.SZ");
    small_tick.price = 100.0;
    small_tick.volume = 999;  // 成交额为100.0 * 999 = 99900.0 < 10万
    small_tick.direction = 'B';

    // 处理小额逐笔成交数据
    processor_->ProcessTickData(small_tick);

    // 获取最新的大单信号
    std::vector<LargeOrderSignal> signals = processor_->GetLatestLargeOrderSignals();

    // 验证是否没有生成大单信号
    ASSERT_EQ(signals.size(), 0);
}

TEST_F(MarketDataProcessorTest, GetOrderBookForNonExistentSymbol) {
    // 获取不存在的标的的订单簿
    OrderBook order_book = processor_->GetOrderBook("NON_EXISTENT_SYMBOL");

    // 验证订单簿是否为空
    ASSERT_TRUE(order_book.bids.empty());
    ASSERT_TRUE(order_book.asks.empty());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
