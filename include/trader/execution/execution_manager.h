/*!
    \file execution_manager.h
    rief Execution manager class
    \author Ivan Shynkarenka
    \date 2024.XX.XX
    \copyright MIT License
*/

#ifndef CPPTRADER_EXECUTION_EXECUTION_MANAGER_H
#define CPPTRADER_EXECUTION_EXECUTION_MANAGER_H

#include "trader/signal/signal_generator.h"
#include "trader/matching/market_manager.h"
#include "trader/matching/order.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace CppTrader {
namespace Execution {

//! Execution manager event handler
class ExecutionHandler
{
public:
    //! Handle execution manager start event
    virtual void onExecutionManagerStart() {}
    //! Handle execution manager stop event
    virtual void onExecutionManagerStop() {}

    //! Handle order created event
    /*!
        \param order - Order
    */
    virtual void onOrderCreated(const Matching::Order& order) {}
    //! Handle order updated event
    /*!
        \param order - Order
    */
    virtual void onOrderUpdated(const Matching::Order& order) {}
    //! Handle order deleted event
    /*!
        \param order - Order
    */
    virtual void onOrderDeleted(const Matching::Order& order) {}

    //! Handle order executed event
    /*!
        \param order - Order
        \param price - Execution price
        \param quantity - Execution quantity
        \param timestamp - Execution timestamp
    */
    virtual void onOrderExecuted(const Matching::Order& order, uint64_t price, uint64_t quantity, uint64_t timestamp) {}

    //! Handle strategy started event
    /*!
        \param symbol - Symbol
    */
    virtual void onStrategyStarted(const Matching::Symbol& symbol) {}
    //! Handle strategy stopped event
    /*!
        \param symbol - Symbol
    */
    virtual void onStrategyStopped(const Matching::Symbol& symbol) {}

    //! Handle big trade signal processed event
    /*!
        \param symbol - Symbol
        \param price - Trade price
        \param quantity - Trade quantity
        \param timestamp - Trade timestamp
        \param amount - Trade amount
    */
    virtual void onBigTradeSignalProcessed(const Matching::Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp, uint64_t amount) {}

    //! Handle error event
    /*!
        \param message - Error message
    */
    virtual void onError(const std::string& message) {}
};

//! Execution manager class
/*!
    Execution manager is used to execute trading strategies based on signals.
    It implements a simple follow strategy that places orders at the current
    best bid/ask price when a big trade signal is received.

    Not thread-safe.
*/
class ExecutionManager : public Signal::SignalHandler, public Matching::MarketHandler
{
public:
    ExecutionManager(Matching::MarketManager& market_manager);
    ExecutionManager(const ExecutionManager&) noexcept = delete;
    ExecutionManager(ExecutionManager&&) noexcept = delete;
    virtual ~ExecutionManager() noexcept;

    ExecutionManager& operator=(const ExecutionManager&) noexcept = delete;
    ExecutionManager& operator=(ExecutionManager&&) noexcept = delete;

    //! Get the execution handler
    ExecutionHandler* handler() noexcept { return _handler; }
    const ExecutionHandler* handler() const noexcept { return _handler; }
    //! Set the execution handler
    void SetHandler(ExecutionHandler* handler) noexcept { _handler = handler; }

    //! Get the market manager
    Matching::MarketManager& market_manager() noexcept { return _market_manager; }
    const Matching::MarketManager& market_manager() const noexcept { return _market_manager; }

    //! Get the follow strategy quantity
    uint64_t follow_quantity() const noexcept { return _follow_quantity; }
    //! Set the follow strategy quantity
    void SetFollowQuantity(uint64_t quantity) noexcept { _follow_quantity = quantity; }

    //! Get the follow strategy slippage
    uint64_t follow_slippage() const noexcept { return _follow_slippage; }
    //! Set the follow strategy slippage
    void SetFollowSlippage(uint64_t slippage) noexcept { _follow_slippage = slippage; }

    //! Start the execution manager
    /*!
        eturn 'true' if the execution manager was successfully started, 'false' if the execution manager failed to start
    */
    bool Start();
    //! Stop the execution manager
    /*!
        eturn 'true' if the execution manager was successfully stopped, 'false' if the execution manager failed to stop
    */
    bool Stop();
    //! Restart the execution manager
    /*!
        eturn 'true' if the execution manager was successfully restarted, 'false' if the execution manager failed to restart
    */
    bool Restart();

    //! Is the execution manager running?
    bool IsRunning() const noexcept { return _running; }

    //! Start the follow strategy for the given symbol
    /*!
        \param symbol - Symbol
        eturn 'true' if the follow strategy was successfully started, 'false' if the follow strategy failed to start
    */
    bool StartFollowStrategy(const Matching::Symbol& symbol);
    //! Stop the follow strategy for the given symbol
    /*!
        \param symbol - Symbol
        eturn 'true' if the follow strategy was successfully stopped, 'false' if the follow strategy failed to stop
    */
    bool StopFollowStrategy(const Matching::Symbol& symbol);

protected:
    // Signal handler implementation
    void onSignalGeneratorStart() override;
    void onSignalGeneratorStop() override;
    void onSymbolAdded(const Matching::Symbol& symbol) override;
    void onSymbolRemoved(const Matching::Symbol& symbol) override;
    void onOrderBookUpdated(const Matching::Symbol& symbol, const Matching::OrderBook& order_book) override;
    void onTrade(const Matching::Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp) override;
    void onBigTradeSignal(const Matching::Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp, uint64_t amount) override;
    void onOrder(const Matching::Symbol& symbol, const Matching::Order& order) override;
    void onExecution(const Matching::Symbol& symbol, const Matching::Order& order, uint64_t price, uint64_t quantity, uint64_t timestamp) override;
    void onError(const std::string& message) override;

    // Market handler implementation
    void onAddSymbol(const Matching::Symbol& symbol) override;
    void onDeleteSymbol(const Matching::Symbol& symbol) override;
    void onAddOrderBook(const Matching::OrderBook& order_book) override;
    void onDeleteOrderBook(const Matching::OrderBook& order_book) override;
    void onAddOrder(const Matching::Order& order) override;
    void onUpdateOrder(const Matching::Order& order) override;
    void onDeleteOrder(const Matching::Order& order) override;
    void onExecuteOrder(const Matching::Order& order, uint64_t price, uint64_t quantity) override;
    void onMatch(const Matching::Order& order1, const Matching::Order& order2, uint64_t price, uint64_t quantity) override;
    void onError(Matching::ErrorCode error, const std::string& message) override;

private:
    struct StrategyState
    {
        bool active{false};
        uint64_t last_signal_time{0};
        uint64_t last_order_id{0};
    };

    Matching::MarketManager& _market_manager;
    ExecutionHandler* _handler{nullptr};
    bool _running{false};
    uint64_t _follow_quantity{100};
    uint64_t _follow_slippage{0};
    uint64_t _next_order_id{1};

    // Strategy states for all symbols
    std::map<uint32_t, StrategyState> _strategy_states;

    // Execute the follow strategy for the given symbol
    void ExecuteFollowStrategy(const Matching::Symbol& symbol, const Matching::OrderBook& order_book, bool buy);
};

} // namespace Execution
} // namespace CppTrader

#include "execution_manager.inl"

#endif // CPPTRADER_EXECUTION_EXECUTION_MANAGER_H
