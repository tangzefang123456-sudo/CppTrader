#include "gtest/gtest.h"
#include "trading_executor.h"
#include <string>

using namespace TradingSystem;

class TradingExecutorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建交易执行器实例
        executor_ = new TradingExecutor();
        ASSERT_NE(executor_, nullptr);

        // 初始化交易执行器
        bool init_result = executor_->Init("test_config");
        ASSERT_TRUE(init_result);
    }

    void TearDown() override {
        // 清空交易执行器
        executor_->Clear();

        // 销毁交易执行器实例
        delete executor_;
        executor_ = nullptr;
    }

    TradingExecutor* executor_;
};

TEST_F(TradingExecutorTest, ProcessLargeOrderSignal) {
    // 创建测试用的大单信号
    LargeOrderSignal signal;
    signal.timestamp = 1234567890123456789ULL;
    signal.symbol = "000001.SZ";
    signal.price = 10.0;
    signal.volume = 1001;
    signal.amount = 100.0 * 1001;
    signal.direction = 'B';

    // 处理大单信号
    executor_->ProcessLargeOrderSignal(signal);

    // 获取所有订单
    std::vector<Order> orders = executor_->GetAllOrders();

    // 验证是否生成了订单
    ASSERT_EQ(orders.size(), 1);

    // 验证订单的内容是否正确
    const Order& order = orders[0];
    ASSERT_EQ(order.timestamp, 1234567890123456789ULL);
    ASSERT_EQ(order.symbol, "000001.SZ");
    ASSERT_EQ(order.order_type, 'B');
    ASSERT_DOUBLE_EQ(order.price, 10.01);  // 买单价格比大单成交价格高0.01
    ASSERT_EQ(order.volume, 100);          // 每次下单数量为100股
    ASSERT_FALSE(order.order_id.empty());
    ASSERT_EQ(order.status, 'P');          // 订单状态为已提交
}

TEST_F(TradingExecutorTest, ProcessLargeSellOrderSignal) {
    // 创建测试用的大单卖出信号
    LargeOrderSignal signal;
    signal.timestamp = 1234567890123456789ULL;
    signal.symbol = "000001.SZ";
    signal.price = 10.0;
    signal.volume = 1001;
    signal.amount = 100.0 * 1001;
    signal.direction = 'S';

    // 处理大单卖出信号
    executor_->ProcessLargeOrderSignal(signal);

    // 获取所有订单
    std::vector<Order> orders = executor_->GetAllOrders();

    // 验证是否生成了订单
    ASSERT_EQ(orders.size(), 1);

    // 验证订单的内容是否正确
    const Order& order = orders[0];
    ASSERT_EQ(order.timestamp, 1234567890123456789ULL);
    ASSERT_EQ(order.symbol, "000001.SZ");
    ASSERT_EQ(order.order_type, 'S');
    ASSERT_DOUBLE_EQ(order.price, 9.99);   // 卖单价格比大单成交价格低0.01
    ASSERT_EQ(order.volume, 100);          // 每次下单数量为100股
    ASSERT_FALSE(order.order_id.empty());
    ASSERT_EQ(order.status, 'P');          // 订单状态为已提交
}

TEST_F(TradingExecutorTest, GetOrderById) {
    // 创建测试用的大单信号
    LargeOrderSignal signal;
    signal.timestamp = 1234567890123456789ULL;
    signal.symbol = "000001.SZ";
    signal.price = 10.0;
    signal.volume = 1001;
    signal.amount = 100.0 * 1001;
    signal.direction = 'B';

    // 处理大单信号
    executor_->ProcessLargeOrderSignal(signal);

    // 获取所有订单
    std::vector<Order> orders = executor_->GetAllOrders();
    ASSERT_EQ(orders.size(), 1);

    // 根据订单ID获取订单
    Order order = executor_->GetOrderById(orders[0].order_id);

    // 验证订单是否正确
    ASSERT_EQ(order.timestamp, 1234567890123456789ULL);
    ASSERT_EQ(order.symbol, "000001.SZ");
    ASSERT_EQ(order.order_type, 'B');
    ASSERT_DOUBLE_EQ(order.price, 10.01);
    ASSERT_EQ(order.volume, 100);
    ASSERT_EQ(order.order_id, orders[0].order_id);
    ASSERT_EQ(order.status, 'P');
}

TEST_F(TradingExecutorTest, GetNonExistentOrderById) {
    // 根据不存在的订单ID获取订单
    Order order = executor_->GetOrderById("NON_EXISTENT_ORDER_ID");

    // 验证返回的订单是否为空
    ASSERT_EQ(order.timestamp, 0);
    ASSERT_EQ(order.symbol, "");
    ASSERT_EQ(order.order_type, ' ');
    ASSERT_DOUBLE_EQ(order.price, 0.0);
    ASSERT_EQ(order.volume, 0);
    ASSERT_EQ(order.order_id, "");
    ASSERT_EQ(order.status, ' ');
}

TEST_F(TradingExecutorTest, CancelOrder) {
    // 创建测试用的大单信号
    LargeOrderSignal signal;
    signal.timestamp = 1234567890123456789ULL;
    signal.symbol = "000001.SZ";
    signal.price = 10.0;
    signal.volume = 1001;
    signal.amount = 100.0 * 1001;
    signal.direction = 'B';

    // 处理大单信号
    executor_->ProcessLargeOrderSignal(signal);

    // 获取所有订单
    std::vector<Order> orders = executor_->GetAllOrders();
    ASSERT_EQ(orders.size(), 1);

    // 撤销订单
    bool cancel_result = executor_->CancelOrder(orders[0].order_id);
    ASSERT_TRUE(cancel_result);

    // 验证订单状态是否已更新为已撤销
    Order canceled_order = executor_->GetOrderById(orders[0].order_id);
    ASSERT_EQ(canceled_order.status, 'C');
}

TEST_F(TradingExecutorTest, CancelNonExistentOrder) {
    // 撤销不存在的订单
    bool cancel_result = executor_->CancelOrder("NON_EXISTENT_ORDER_ID");
    ASSERT_FALSE(cancel_result);
}

TEST_F(TradingExecutorTest, CancelAlreadyCanceledOrder) {
    // 创建测试用的大单信号
    LargeOrderSignal signal;
    signal.timestamp = 1234567890123456789ULL;
    signal.symbol = "000001.SZ";
    signal.price = 10.0;
    signal.volume = 1001;
    signal.amount = 100.0 * 1001;
    signal.direction = 'B';

    // 处理大单信号
    executor_->ProcessLargeOrderSignal(signal);

    // 获取所有订单
    std::vector<Order> orders = executor_->GetAllOrders();
    ASSERT_EQ(orders.size(), 1);

    // 第一次撤销订单
    bool cancel_result1 = executor_->CancelOrder(orders[0].order_id);
    ASSERT_TRUE(cancel_result1);

    // 第二次撤销同一订单
    bool cancel_result2 = executor_->CancelOrder(orders[0].order_id);
    ASSERT_FALSE(cancel_result2);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
