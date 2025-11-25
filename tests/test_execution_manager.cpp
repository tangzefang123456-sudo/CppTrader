#include "test.h"
#include "trader/execution/execution_manager.h"

using namespace CppTrader;

// 测试执行处理器
class TestExecutionHandler : public Execution::ExecutionHandler {
public:
    TestExecutionHandler() : _last_order_id(0) {}

    uint64_t SubmitOrder(const Execution::OrderRequest& request) override {
        _last_order_request = request;
        _last_order_id++;
        return _last_order_id;
    }

    bool CancelOrder(uint64_t order_id) override {
        _cancelled_order_id = order_id;
        return true;
    }

    bool QueryOrder(uint64_t order_id, Execution::OrderResult& result) override {
        _queried_order_id = order_id;
        return false;
    }

    Execution::OrderRequest GetLastOrderRequest() const { return _last_order_request; }
    uint64_t GetLastOrderId() const { return _last_order_id; }
    uint64_t GetCancelledOrderId() const { return _cancelled_order_id; }
    uint64_t GetQueriedOrderId() const { return _queried_order_id; }

private:
    Execution::OrderRequest _last_order_request;
    uint64_t _last_order_id;
    uint64_t _cancelled_order_id;
    uint64_t _queried_order_id;
};

TEST_CASE("ExecutionManager", "[Execution]")
{
    // 创建测试执行处理器
    auto test_execution_handler = std::make_unique<TestExecutionHandler>();
    auto execution_manager = std::make_unique<Execution::ExecutionManager>(std::move(test_execution_handler));

    // 测试策略参数设置
    double price_offset = 0.0;
    int64_t quantity = 0;
    execution_manager->GetFollowStrategyParams(price_offset, quantity);
    REQUIRE(price_offset == 0.01);
    REQUIRE(quantity == 100);

    execution_manager->SetFollowStrategyParams(0.05, 500);
    execution_manager->GetFollowStrategyParams(price_offset, quantity);
    REQUIRE(price_offset == 0.05);
    REQUIRE(quantity == 500);

    // 测试买入大单信号处理
    SignalGenerator::LargeOrderSignal buy_signal;
    buy_signal.timestamp = 1234567890;
    buy_signal.symbol = "AAPL";
    buy_signal.price = 150.0;
    buy_signal.volume = 1000;
    buy_signal.amount = 150000.0;
    buy_signal.direction = 'B';

    execution_manager->OnLargeOrderSignal(buy_signal);

    // 检查是否提交了正确的追随订单
    auto& handler = static_cast<TestExecutionHandler&>(*execution_manager->_execution_handler);
    Execution::OrderRequest order_request = handler.GetLastOrderRequest();
    REQUIRE(order_request.symbol == "AAPL");
    REQUIRE(order_request.side == 'B');
    REQUIRE(order_request.price == 150.0 + 0.05); // 买入信号，价格+偏移
    REQUIRE(order_request.quantity == 500);

    // 测试卖出大单信号处理
    SignalGenerator::LargeOrderSignal sell_signal;
    sell_signal.timestamp = 1234567891;
    sell_signal.symbol = "GOOG";
    sell_signal.price = 2800.0;
    sell_signal.volume = 50;
    sell_signal.amount = 140000.0;
    sell_signal.direction = 'S';

    execution_manager->OnLargeOrderSignal(sell_signal);

    order_request = handler.GetLastOrderRequest();
    REQUIRE(order_request.symbol == "GOOG");
    REQUIRE(order_request.side == 'S');
    REQUIRE(order_request.price == 2800.0 - 0.05); // 卖出信号，价格-偏移
    REQUIRE(order_request.quantity == 500);
}
