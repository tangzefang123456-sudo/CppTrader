/*!
    \file trading_system_demo.cpp
    \brief Trading system demo
    \author Ivan Shynkarenka
    \date 2024.XX.XX
    \copyright MIT License
*/

#include "trader/market_data/market_data_handler.h"
#include "trader/market_data/market_data_recorder.h"
#include "trader/market_data/market_data_player.h"
#include "trader/signal/signal_generator.h"
#include "trader/execution/execution_manager.h"
#include "trader/matching/market_manager.h"
#include "trader/matching/symbol.h"
#include "trader/matching/order.h"
#include "trader/matching/level.h"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace CppTrader;
using namespace CppTrader::MarketData;
using namespace CppTrader::Signal;
using namespace CppTrader::Execution;
using namespace CppTrader::Matching;

class DemoMarketDataHandler : public MarketDataHandler
{
public:
    DemoMarketDataHandler(MarketDataHandler::Handler& handler)
        : _handler(handler)
    {
    }

protected:
    void onMarketDataStart() override
    {
        std::cout << "Market data handler started!" << std::endl;
        _handler.onMarketDataStart();
    }

    void onMarketDataStop() override
    {
        std::cout << "Market data handler stopped!" << std::endl;
        _handler.onMarketDataStop();
    }

    void onMarketDataSymbol(const Symbol& symbol) override
    {
        std::cout << "Symbol added: " << symbol.ToString() << std::endl;
        _handler.onMarketDataSymbol(symbol);
    }

    void onMarketDataOrderBookUpdate(const Symbol& symbol, const LevelUpdate& update) override
    {
        _handler.onMarketDataOrderBookUpdate(symbol, update);
    }

    void onMarketDataTrade(const Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp) override
    {
        _handler.onMarketDataTrade(symbol, price, quantity, timestamp);
    }

    void onMarketDataOrder(const Symbol& symbol, const Order& order) override
    {
        _handler.onMarketDataOrder(symbol, order);
    }

    void onMarketDataExecution(const Symbol& symbol, const Order& order, uint64_t price, uint64_t quantity, uint64_t timestamp) override
    {
        _handler.onMarketDataExecution(symbol, order, price, quantity, timestamp);
    }

    void onMarketDataError(const std::string& message) override
    {
        std::cout << "Market data error: " << message << std::endl;
        _handler.onMarketDataError(message);
    }

private:
    MarketDataHandler::Handler& _handler;
};

class DemoSignalHandler : public SignalHandler
{
public:
    DemoSignalHandler(ExecutionManager& execution_manager)
        : _execution_manager(execution_manager)
    {
    }

protected:
    void onSignalGeneratorStart() override
    {
        std::cout << "Signal generator started!" << std::endl;
    }

    void onSignalGeneratorStop() override
    {
        std::cout << "Signal generator stopped!" << std::endl;
    }

    void onSymbolAdded(const Symbol& symbol) override
    {
        std::cout << "Symbol added to signal generator: " << symbol.ToString() << std::endl;
        _execution_manager.onMarketDataSymbol(symbol);
    }

    void onSymbolRemoved(const Symbol& symbol) override
    {
        std::cout << "Symbol removed from signal generator: " << symbol.ToString() << std::endl;
        _execution_manager.onMarketDataSymbolRemoved(symbol);
    }

    void onOrderBookUpdated(const Symbol& symbol, const OrderBook& order_book) override
    {
        _execution_manager.onMarketDataOrderBookUpdate(symbol, LevelUpdate()); // Simplified
    }

    void onTrade(const Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp) override
    {
        _execution_manager.onMarketDataTrade(symbol, price, quantity, timestamp);
    }

    void onBigTradeSignal(const Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp, uint64_t amount) override
    {
        std::cout << "Big trade signal generated! Symbol: " << symbol.ToString() << 
                     ", Price: " << price << ", Quantity: " << quantity << 
                     ", Amount: " << amount << std::endl;
        _execution_manager.onBigTradeSignal(symbol, price, quantity, timestamp, amount);
    }

    void onOrder(const Symbol& symbol, const Order& order) override
    {
        _execution_manager.onMarketDataOrder(symbol, order);
    }

    void onExecution(const Symbol& symbol, const Order& order, uint64_t price, uint64_t quantity, uint64_t timestamp) override
    {
        _execution_manager.onMarketDataExecution(symbol, order, price, quantity, timestamp);
    }

    void onError(const std::string& message) override
    {
        std::cout << "Signal generator error: " << message << std::endl;
        _execution_manager.onMarketDataError(message);
    }

private:
    ExecutionManager& _execution_manager;
};

class DemoExecutionHandler : public ExecutionHandler
{
protected:
    void onExecutionManagerStart() override
    {
        std::cout << "Execution manager started!" << std::endl;
    }

    void onExecutionManagerStop() override
    {
        std::cout << "Execution manager stopped!" << std::endl;
    }

    void onSymbolAdded(const Symbol& symbol) override
    {
        std::cout << "Symbol added to execution manager: " << symbol.ToString() << std::endl;
    }

    void onSymbolRemoved(const Symbol& symbol) override
    {
        std::cout << "Symbol removed from execution manager: " << symbol.ToString() << std::endl;
    }

    void onOrderBookUpdated(const Symbol& symbol, const OrderBook& order_book) override
    {
        // Not used in this demo
    }

    void onTrade(const Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp) override
    {
        // Not used in this demo
    }

    void onOrder(const Symbol& symbol, const Order& order) override
    {
        // Not used in this demo
    }

    void onExecution(const Symbol& symbol, const Order& order, uint64_t price, uint64_t quantity, uint64_t timestamp) override
    {
        // Not used in this demo
    }

    void onOrderAdded(const Symbol& symbol, const Order& order) override
    {
        std::cout << "Order added: " << order.ToString() << " for symbol: " << symbol.ToString() << std::endl;
    }

    void onOrderCanceled(const Symbol& symbol, const Order& order) override
    {
        std::cout << "Order canceled: " << order.ToString() << " for symbol: " << symbol.ToString() << std::endl;
    }

    void onOrderReplaced(const Symbol& symbol, const Order& order) override
    {
        std::cout << "Order replaced: " << order.ToString() << " for symbol: " << symbol.ToString() << std::endl;
    }

    void onError(const std::string& message) override
    {
        std::cout << "Execution manager error: " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    std::cout << "Trading System Demo" << std::endl;
    std::cout << "==================" << std::endl;

    try
    {
        // Create market manager
        MarketManager market_manager;

        // Create execution manager
        ExecutionManager execution_manager(market_manager);
        DemoExecutionHandler execution_handler;
        execution_manager.SetHandler(&execution_handler);

        // Create signal generator
        SignalGenerator signal_generator(market_manager);
        DemoSignalHandler signal_handler(execution_manager);
        signal_generator.SetHandler(&signal_handler);

        // Create market data recorder
        MarketDataRecorder recorder("market_data.bin");

        // Create demo market data handler
        DemoMarketDataHandler market_data_handler(recorder);

        // Start all components
        execution_manager.Start();
        signal_generator.Start();
        market_data_handler.Start();

        // Simulate market data
        Symbol symbol(1, "AAPL");
        market_data_handler.onMarketDataSymbol(symbol);

        // Add order book levels
        Level bid_level(LevelType::BID, 1000000, 1000, 0, 1000, 1);
        Level ask_level(LevelType::ASK, 1000100, 1000, 0, 1000, 1);
        LevelUpdate bid_update(UpdateType::ADD, bid_level, true);
        LevelUpdate ask_update(UpdateType::ADD, ask_level, true);
        market_data_handler.onMarketDataOrderBookUpdate(symbol, bid_update);
        market_data_handler.onMarketDataOrderBookUpdate(symbol, ask_update);

        // Simulate small trades (should not generate signals)
        uint64_t timestamp = CppCommon::Timestamp::Now().timestamp();
        for (int i = 0; i < 5; ++i)
        {
            market_data_handler.onMarketDataTrade(symbol, 1000000, 50, timestamp); // Amount = 50,000,000
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Simulate a big trade (should generate a signal)
        std::cout << std::endl << "Simulating big trade..." << std::endl;
        market_data_handler.onMarketDataTrade(symbol, 1000000, 150, timestamp); // Amount = 150,000,000
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Simulate another big trade on the ask side
        std::cout << std::endl << "Simulating big trade on ask side..." << std::endl;
        market_data_handler.onMarketDataTrade(symbol, 1000100, 200, timestamp); // Amount = 200,020,000
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Stop all components
        market_data_handler.Stop();
        signal_generator.Stop();
        execution_manager.Stop();

        std::cout << std::endl << "Demo completed successfully!" << std::endl;

        // Now demonstrate market data playback
        std::cout << std::endl << "Market Data Playback Demo" << std::endl;
        std::cout << "=========================" << std::endl;

        // Create market data player
        MarketDataPlayer player("market_data.bin");
        player.SetHandler(&signal_handler);

        // Start playback
        player.Start();
        player.Play();

        // Wait for playback to complete
        while (player.IsPlaying())
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Stop player
        player.Stop();

        std::cout << std::endl << "Playback completed successfully!" << std::endl;

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
