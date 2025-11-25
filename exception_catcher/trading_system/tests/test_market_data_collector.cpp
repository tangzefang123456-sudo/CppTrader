#include "gtest/gtest.h"
#include "market_data_collector.h"
#include <string>
#include <fstream>

using namespace TradingSystem;

class MarketDataCollectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建行情数据收集器实例
        collector_ = CreateMarketDataCollector();
        ASSERT_NE(collector_, nullptr);

        // 初始化行情数据收集器
        bool init_result = collector_->Init("test_config");
        ASSERT_TRUE(init_result);

        // 开始行情数据收集
        bool start_result = collector_->Start();
        ASSERT_TRUE(start_result);
    }

    void TearDown() override {
        // 停止行情数据收集
        bool stop_result = collector_->Stop();
        ASSERT_TRUE(stop_result);

        // 销毁行情数据收集器实例
        DestroyMarketDataCollector(collector_);
        collector_ = nullptr;

        // 删除测试文件
        std::remove(test_file_path_.c_str());
    }

    MarketDataCollector* collector_;
    const std::string test_file_path_ = "test_market_data.bin";
};

TEST_F(MarketDataCollectorTest, ProcessTickData) {
    // 创建测试用的逐笔成交数据
    TickData tick;
    tick.timestamp = 1234567890123456789ULL;
    tick.symbol = "000001.SZ";
    tick.price = 10.0;
    tick.volume = 100;
    tick.direction = 'B';

    // 处理逐笔成交数据
    collector_->OnTickData(tick);

    // 这里可以添加更多的验证逻辑，比如检查共享内存中的数据
}

TEST_F(MarketDataCollectorTest, ProcessOrderData) {
    // 创建测试用的逐笔委托数据
    OrderData order;
    order.timestamp = 1234567890123456789ULL;
    order.symbol = "000001.SZ";
    order.price = 10.0;
    order.volume = 100;
    order.order_type = 'B';
    order.order_id = "ORDER000001";

    // 处理逐笔委托数据
    collector_->OnOrderData(order);

    // 这里可以添加更多的验证逻辑，比如检查共享内存中的数据
}

TEST_F(MarketDataCollectorTest, SaveToBinaryFile) {
    // 创建测试用的逐笔成交数据
    TickData tick;
    tick.timestamp = 1234567890123456789ULL;
    tick.symbol = "000001.SZ";
    tick.price = 10.0;
    tick.volume = 100;
    tick.direction = 'B';

    // 处理逐笔成交数据
    collector_->OnTickData(tick);

    // 创建测试用的逐笔委托数据
    OrderData order;
    order.timestamp = 1234567890123456789ULL;
    order.symbol = "000001.SZ";
    order.price = 10.0;
    order.volume = 100;
    order.order_type = 'B';
    order.order_id = "ORDER000001";

    // 处理逐笔委托数据
    collector_->OnOrderData(order);

    // 保存行情数据到本地二进制文件
    bool save_result = collector_->SaveToBinaryFile(test_file_path_);
    ASSERT_TRUE(save_result);

    // 检查文件是否存在
    std::ifstream in_file(test_file_path_);
    ASSERT_TRUE(in_file.good());
    in_file.close();
}

TEST_F(MarketDataCollectorTest, LoadFromBinaryFile) {
    // 创建测试用的逐笔成交数据
    TickData tick;
    tick.timestamp = 1234567890123456789ULL;
    strcpy_s(tick.symbol, "000001.SZ");
    tick.price = 10.0;
    tick.volume = 100;
    tick.direction = 'B';

    // 处理逐笔成交数据
    collector_->OnTickData(tick);

    // 创建测试用的逐笔委托数据
    OrderData order;
    order.timestamp = 1234567890123456789ULL;
    strcpy_s(order.symbol, "000001.SZ");
    order.price = 10.0;
    order.volume = 100;
    order.order_type = 'B';
    strcpy_s(order.order_id, "ORDER000001");

    // 处理逐笔委托数据
    collector_->OnOrderData(order);

    // 保存行情数据到本地二进制文件
    bool save_result = collector_->SaveToBinaryFile(test_file_path_);
    ASSERT_TRUE(save_result);

    // 停止行情数据收集
    bool stop_result = collector_->Stop();
    ASSERT_TRUE(stop_result);

    // 重新开始行情数据收集
    bool start_result = collector_->Start();
    ASSERT_TRUE(start_result);

    // 从本地二进制文件加载行情数据并模拟播放
    bool load_result = collector_->LoadFromBinaryFile(test_file_path_);
    ASSERT_TRUE(load_result);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
