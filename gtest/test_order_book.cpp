// Google Test 单元测试文件
// 用于测试 OrderBook 类的功能

#include <gtest/gtest.h>
#include "trader/matching/order_book.h"
#include "trader/matching/market_manager.h"
#include "trader/matching/symbol.h"
#include "trader/matching/order.h"

using namespace CppTrader::Matching;

// 测试环境类，用于设置和清理测试所需的资源
class OrderBookTest : public ::testing::Test {
protected:
    MarketManager market_manager;
    Symbol symbol;
    OrderBook* order_book;

    void SetUp() override {
        // 创建一个测试用的 Symbol
        symbol = Symbol(1, "TEST", 2, 2);
        
        // 添加 Symbol 到 MarketManager
        market_manager.AddSymbol(symbol);
        
        // 添加 OrderBook 到 MarketManager
        market_manager.AddOrderBook(symbol);
        
        // 获取 OrderBook 指针
        order_book = const_cast<OrderBook*>(market_manager.GetOrderBook(symbol.Id));
    }

    void TearDown() override {
        // 清理资源
        market_manager.DeleteOrderBook(symbol.Id);
        market_manager.DeleteSymbol(symbol.Id);
    }
};

// 测试 OrderBook 构造函数和基本属性
TEST_F(OrderBookTest, ConstructorAndBasicProperties) {
    EXPECT_EQ(order_book->symbol().Id, symbol.Id);
    EXPECT_EQ(order_book->symbol().Name, symbol.Name);
    EXPECT_TRUE(order_book->empty());
    EXPECT_EQ(order_book->size(), 0);
    EXPECT_EQ(order_book->best_bid(), nullptr);
    EXPECT_EQ(order_book->best_ask(), nullptr);
}

// 测试添加和删除限价订单
TEST_F(OrderBookTest, AddAndDeleteLimitOrder) {
    // 创建一个买单
    Order buy_order = Order::BuyLimit(1, symbol.Id, 100, 10);
    OrderNode buy_order_node(buy_order);
    
    // 添加买单到 OrderBook
    LevelUpdate update = order_book->AddOrder(&buy_order_node);
    
    // 验证订单已添加
    EXPECT_FALSE(order_book->empty());
    EXPECT_EQ(order_book->size(), 1);
    EXPECT_NE(order_book->best_bid(), nullptr);
    EXPECT_EQ(order_book->best_bid()->Price, 100);
    EXPECT_EQ(order_book->best_bid()->TotalVolume, 10);
    
    // 创建一个卖单
    Order sell_order = Order::SellLimit(2, symbol.Id, 110, 10);
    OrderNode sell_order_node(sell_order);
    
    // 添加卖单到 OrderBook
    update = order_book->AddOrder(&sell_order_node);
    
    // 验证卖单已添加
    EXPECT_EQ(order_book->size(), 2);
    EXPECT_NE(order_book->best_ask(), nullptr);
    EXPECT_EQ(order_book->best_ask()->Price, 110);
    EXPECT_EQ(order_book->best_ask()->TotalVolume, 10);
    
    // 删除买单
    update = order_book->DeleteOrder(&buy_order_node);
    
    // 验证买单已删除
    EXPECT_EQ(order_book->size(), 1);
    EXPECT_EQ(order_book->best_bid(), nullptr);
    
    // 删除卖单
    update = order_book->DeleteOrder(&sell_order_node);
    
    // 验证卖单已删除
    EXPECT_TRUE(order_book->empty());
    EXPECT_EQ(order_book->size(), 0);
    EXPECT_EQ(order_book->best_ask(), nullptr);
}

// 测试修改订单数量
TEST_F(OrderBookTest, ReduceOrderQuantity) {
    // 创建一个订单
    Order order = Order::BuyLimit(1, symbol.Id, 100, 10);
    OrderNode order_node(order);
    
    // 添加订单到 OrderBook
    order_book->AddOrder(&order_node);
    
    // 验证初始数量
    EXPECT_EQ(order_book->best_bid()->TotalVolume, 10);
    
    // 减少订单数量
    LevelUpdate update = order_book->ReduceOrder(&order_node, 5, 0, 5);
    order_node.LeavesQuantity = 5;
    
    // 验证数量已减少
    EXPECT_EQ(order_book->best_bid()->TotalVolume, 5);
    
    // 完全减少订单数量
    update = order_book->ReduceOrder(&order_node, 5, 0, 5);
    order_node.LeavesQuantity = 0;
    
    // 验证订单已删除
    EXPECT_TRUE(order_book->empty());
    EXPECT_EQ(order_book->best_bid(), nullptr);
}

// 测试添加多个相同价格的订单
TEST_F(OrderBookTest, AddMultipleOrdersAtSamePrice) {
    // 创建第一个订单
    Order order1 = Order::BuyLimit(1, symbol.Id, 100, 10);
    OrderNode order_node1(order1);
    
    // 添加第一个订单
    order_book->AddOrder(&order_node1);
    
    // 验证第一个订单
    EXPECT_EQ(order_book->best_bid()->TotalVolume, 10);
    EXPECT_EQ(order_book->best_bid()->Orders, 1);
    
    // 创建第二个订单，相同价格
    Order order2 = Order::BuyLimit(2, symbol.Id, 100, 15);
    OrderNode order_node2(order2);
    
    // 添加第二个订单
    order_book->AddOrder(&order_node2);
    
    // 验证两个订单在同一价格级别
    EXPECT_EQ(order_book->best_bid()->TotalVolume, 25); // 10 + 15
    EXPECT_EQ(order_book->best_bid()->Orders, 2);
    
    // 删除第一个订单
    order_book->DeleteOrder(&order_node1);
    
    // 验证还剩一个订单
    EXPECT_EQ(order_book->best_bid()->TotalVolume, 15);
    EXPECT_EQ(order_book->best_bid()->Orders, 1);
    
    // 删除第二个订单
    order_book->DeleteOrder(&order_node2);
    
    // 验证订单簿为空
    EXPECT_TRUE(order_book->empty());
}

// 测试添加不同价格的订单
TEST_F(OrderBookTest, AddOrdersAtDifferentPrices) {
    // 创建三个不同价格的买单
    Order order1 = Order::BuyLimit(1, symbol.Id, 100, 10); // 最低价格
    Order order2 = Order::BuyLimit(2, symbol.Id, 105, 15); // 中间价格
    Order order3 = Order::BuyLimit(3, symbol.Id, 110, 20); // 最高价格
    
    OrderNode order_node1(order1);
    OrderNode order_node2(order2);
    OrderNode order_node3(order3);
    
    // 按价格从低到高添加订单
    order_book->AddOrder(&order_node1);
    order_book->AddOrder(&order_node2);
    order_book->AddOrder(&order_node3);
    
    // 验证最佳买价是最高价格
    EXPECT_EQ(order_book->best_bid()->Price, 110);
    EXPECT_EQ(order_book->best_bid()->TotalVolume, 20);
    
    // 删除最高价格的订单
    order_book->DeleteOrder(&order_node3);
    
    // 验证最佳买价现在是中间价格
    EXPECT_EQ(order_book->best_bid()->Price, 105);
    EXPECT_EQ(order_book->best_bid()->TotalVolume, 15);
    
    // 删除中间价格的订单
    order_book->DeleteOrder(&order_node2);
    
    // 验证最佳买价现在是最低价格
    EXPECT_EQ(order_book->best_bid()->Price, 100);
    EXPECT_EQ(order_book->best_bid()->TotalVolume, 10);
}

// 测试止损订单功能
TEST_F(OrderBookTest, AddAndDeleteStopOrder) {
    // 创建止损买单
    Order stop_buy_order = Order::BuyStop(1, symbol.Id, 120, 10);
    OrderNode stop_buy_order_node(stop_buy_order);
    
    // 添加止损买单
    order_book->AddStopOrder(&stop_buy_order_node);
    
    // 验证止损订单已添加
    EXPECT_FALSE(order_book->empty());
    EXPECT_EQ(order_book->size(), 1);
    EXPECT_NE(order_book->best_buy_stop(), nullptr);
    EXPECT_EQ(order_book->best_buy_stop()->Price, 120);
    
    // 创建止损卖单
    Order stop_sell_order = Order::SellStop(2, symbol.Id, 90, 10);
    OrderNode stop_sell_order_node(stop_sell_order);
    
    // 添加止损卖单
    order_book->AddStopOrder(&stop_sell_order_node);
    
    // 验证止损卖单已添加
    EXPECT_EQ(order_book->size(), 2);
    EXPECT_NE(order_book->best_sell_stop(), nullptr);
    EXPECT_EQ(order_book->best_sell_stop()->Price, 90);
    
    // 删除止损买单
    order_book->DeleteStopOrder(&stop_buy_order_node);
    
    // 验证止损买单已删除
    EXPECT_EQ(order_book->size(), 1);
    EXPECT_EQ(order_book->best_buy_stop(), nullptr);
    
    // 删除止损卖单
    order_book->DeleteStopOrder(&stop_sell_order_node);
    
    // 验证止损卖单已删除
    EXPECT_TRUE(order_book->empty());
    EXPECT_EQ(order_book->best_sell_stop(), nullptr);
}

// 测试追踪止损订单功能
TEST_F(OrderBookTest, AddAndDeleteTrailingStopOrder) {
    // 创建追踪止损买单
    Order trailing_stop_buy_order = Order::TrailingBuyStop(1, symbol.Id, 120, 10, 5);
    OrderNode trailing_stop_buy_order_node(trailing_stop_buy_order);
    
    // 添加追踪止损买单
    order_book->AddTrailingStopOrder(&trailing_stop_buy_order_node);
    
    // 验证追踪止损订单已添加
    EXPECT_FALSE(order_book->empty());
    EXPECT_EQ(order_book->size(), 1);
    EXPECT_NE(order_book->best_trailing_buy_stop(), nullptr);
    EXPECT_EQ(order_book->best_trailing_buy_stop()->Price, 120);
    
    // 创建追踪止损卖单
    Order trailing_stop_sell_order = Order::TrailingSellStop(2, symbol.Id, 90, 10, 5);
    OrderNode trailing_stop_sell_order_node(trailing_stop_sell_order);
    
    // 添加追踪止损卖单
    order_book->AddTrailingStopOrder(&trailing_stop_sell_order_node);
    
    // 验证追踪止损卖单已添加
    EXPECT_EQ(order_book->size(), 2);
    EXPECT_NE(order_book->best_trailing_sell_stop(), nullptr);
    EXPECT_EQ(order_book->best_trailing_sell_stop()->Price, 90);
    
    // 删除追踪止损买单
    order_book->DeleteTrailingStopOrder(&trailing_stop_buy_order_node);
    
    // 验证追踪止损买单已删除
    EXPECT_EQ(order_book->size(), 1);
    EXPECT_EQ(order_book->best_trailing_buy_stop(), nullptr);
    
    // 删除追踪止损卖单
    order_book->DeleteTrailingStopOrder(&trailing_stop_sell_order_node);
    
    // 验证追踪止损卖单已删除
    EXPECT_TRUE(order_book->empty());
    EXPECT_EQ(order_book->best_trailing_sell_stop(), nullptr);
}

// 测试计算追踪止损价格
TEST_F(OrderBookTest, CalculateTrailingStopPrice) {
    // 创建一个限价卖单来设置市场价格
    Order sell_order = Order::SellLimit(1, symbol.Id, 100, 10);
    OrderNode sell_order_node(sell_order);
    order_book->AddOrder(&sell_order_node);
    
    // 创建追踪止损卖单
    Order trailing_stop_order = Order::TrailingSellStop(2, symbol.Id, 95, 10, 5); // 5点追踪距离
    OrderNode trailing_stop_order_node(trailing_stop_order);
    
    // 添加一些交易来模拟市场价格变化
    // 假设市场价格上涨到110
    order_book->UpdateMatchingPrice(trailing_stop_order, 110);
    
    // 计算新的追踪止损价格
    uint64_t new_stop_price = order_book->CalculateTrailingStopPrice(trailing_stop_order);
    
    // 验证止损价格已调整
    EXPECT_EQ(new_stop_price, 105); // 110 - 5 = 105
    
    // 清理
    order_book->DeleteOrder(&sell_order_node);
}

// 测试获取价格级别
TEST_F(OrderBookTest, GetPriceLevels) {
    // 创建多个不同价格的订单
    Order order1 = Order::BuyLimit(1, symbol.Id, 100, 10);
    Order order2 = Order::BuyLimit(2, symbol.Id, 105, 15);
    Order order3 = Order::SellLimit(3, symbol.Id, 110, 20);
    Order order4 = Order::SellLimit(4, symbol.Id, 115, 25);
    
    OrderNode order_node1(order1);
    OrderNode order_node2(order2);
    OrderNode order_node3(order3);
    OrderNode order_node4(order4);
    
    // 添加订单
    order_book->AddOrder(&order_node1);
    order_book->AddOrder(&order_node2);
    order_book->AddOrder(&order_node3);
    order_book->AddOrder(&order_node4);
    
    // 验证可以获取到正确的价格级别
    EXPECT_NE(order_book->GetBid(100), nullptr);
    EXPECT_NE(order_book->GetBid(105), nullptr);
    EXPECT_NE(order_book->GetAsk(110), nullptr);
    EXPECT_NE(order_book->GetAsk(115), nullptr);
    
    // 验证不存在的价格级别返回nullptr
    EXPECT_EQ(order_book->GetBid(99), nullptr);
    EXPECT_EQ(order_book->GetAsk(120), nullptr);
    
    // 清理
    order_book->DeleteOrder(&order_node1);
    order_book->DeleteOrder(&order_node2);
    order_book->DeleteOrder(&order_node3);
    order_book->DeleteOrder(&order_node4);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
