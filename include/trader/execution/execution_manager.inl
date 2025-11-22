/*!
    \file execution_manager.inl
    rief Execution manager inline implementation
    \author Ivan Shynkarenka
    \date 2024.XX.XX
    \copyright MIT License
*/

namespace CppTrader {
namespace Execution {

inline ExecutionManager::ExecutionManager(Matching::MarketManager& market_manager)
    : _market_manager(market_manager)
{
    // Set the market handler
    _market_manager.SetHandler(this);
}

inline ExecutionManager::~ExecutionManager() noexcept
{
    Stop();
}

inline bool ExecutionManager::Start()
{
    if (IsRunning())
        return false;

    _running = true;
    if (_handler)
        _handler->onExecutionManagerStart();
    return true;
}

inline bool ExecutionManager::Stop()
{
    if (!IsRunning())
        return false;

    _running = false;
    if (_handler)
        _handler->onExecutionManagerStop();
    return true;
}

inline bool ExecutionManager::Restart()
{
    if (IsRunning())
        Stop();
    return Start();
}

inline bool ExecutionManager::StartFollowStrategy(const Matching::Symbol& symbol)
{
    StrategyState& state = _strategy_states[symbol.Id];
    if (state.active)
        return false;

    state.active = true;
    if (_handler)
        _handler->onStrategyStarted(symbol);
    return true;
}

inline bool ExecutionManager::StopFollowStrategy(const Matching::Symbol& symbol)
{
    auto it = _strategy_states.find(symbol.Id);
    if (it == _strategy_states.end() || !it->second.active)
        return false;

    it->second.active = false;
    if (_handler)
        _handler->onStrategyStopped(symbol);
    return true;
}

inline void ExecutionManager::onSignalGeneratorStart()
{
    Start();
}

inline void ExecutionManager::onSignalGeneratorStop()
{
    Stop();
}

inline void ExecutionManager::onSymbolAdded(const Matching::Symbol& symbol)
{
    // Start the follow strategy for the new symbol
    StartFollowStrategy(symbol);
}

inline void ExecutionManager::onSymbolRemoved(const Matching::Symbol& symbol)
{
    // Stop the follow strategy for the removed symbol
    StopFollowStrategy(symbol);
}

inline void ExecutionManager::onOrderBookUpdated(const Matching::Symbol& symbol, const Matching::OrderBook& order_book)
{
    // Not implemented yet
}

inline void ExecutionManager::onTrade(const Matching::Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp)
{
    // Not implemented yet
}

inline void ExecutionManager::onBigTradeSignal(const Matching::Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp, uint64_t amount)
{
    // Find the strategy state for the symbol
    auto it = _strategy_states.find(symbol.Id);
    if (it == _strategy_states.end() || !it->second.active)
        return;

    StrategyState& state = it->second;

    // Update the last signal time
    state.last_signal_time = timestamp;

    // Get the order book for the symbol
    Matching::OrderBook* order_book = _market_manager.GetOrderBook(symbol.Id);
    if (order_book == nullptr)
        return;

    // Determine the direction of the trade (buy or sell)
    // For simplicity, we'll follow the direction of the big trade
    // In a real implementation, we might want to analyze the order book more carefully
    bool buy = (quantity > 0); // Assuming positive quantity means buy

    // Execute the follow strategy
    ExecuteFollowStrategy(symbol, *order_book, buy);

    // Notify that the big trade signal has been processed
    if (_handler)
        _handler->onBigTradeSignalProcessed(symbol, price, quantity, timestamp, amount);
}

inline void ExecutionManager::onOrder(const Matching::Symbol& symbol, const Matching::Order& order)
{
    // Not implemented yet
}

inline void ExecutionManager::onExecution(const Matching::Symbol& symbol, const Matching::Order& order, uint64_t price, uint64_t quantity, uint64_t timestamp)
{
    // Not implemented yet
}

inline void ExecutionManager::onError(const std::string& message)
{
    if (_handler)
        _handler->onError(message);
}

inline void ExecutionManager::onAddSymbol(const Matching::Symbol& symbol)
{
    // Not implemented yet
}

inline void ExecutionManager::onDeleteSymbol(const Matching::Symbol& symbol)
{
    // Not implemented yet
}

inline void ExecutionManager::onAddOrderBook(const Matching::OrderBook& order_book)
{
    // Not implemented yet
}

inline void ExecutionManager::onDeleteOrderBook(const Matching::OrderBook& order_book)
{
    // Not implemented yet
}

inline void ExecutionManager::onAddOrder(const Matching::Order& order)
{
    if (_handler)
        _handler->onOrderCreated(order);
}

inline void ExecutionManager::onUpdateOrder(const Matching::Order& order)
{
    if (_handler)
        _handler->onOrderUpdated(order);
}

inline void ExecutionManager::onDeleteOrder(const Matching::Order& order)
{
    if (_handler)
        _handler->onOrderDeleted(order);
}

inline void ExecutionManager::onExecuteOrder(const Matching::Order& order, uint64_t price, uint64_t quantity)
{
    if (_handler)
        _handler->onOrderExecuted(order, price, quantity, CppCommon::Timestamp::Now().timestamp());
}

inline void ExecutionManager::onMatch(const Matching::Order& order1, const Matching::Order& order2, uint64_t price, uint64_t quantity)
{
    // Not implemented yet
}

inline void ExecutionManager::onError(Matching::ErrorCode error, const std::string& message)
{
    if (_handler)
        _handler->onError(message);
}

inline void ExecutionManager::ExecuteFollowStrategy(const Matching::Symbol& symbol, const Matching::OrderBook& order_book, bool buy)
{
    // Get the best bid/ask price
    uint64_t price = 0;
    if (buy)
    {
        const Matching::LevelNode* best_ask = order_book.best_ask();
        if (best_ask == nullptr)
            return;
        price = best_ask->Price;
    }
    else
    {
        const Matching::LevelNode* best_bid = order_book.best_bid();
        if (best_bid == nullptr)
            return;
        price = best_bid->Price;
    }

    // Adjust price for slippage
    if (buy && (price <= (std::numeric_limits<uint64_t>::max() - _follow_slippage)))
        price += _follow_slippage;
    else if (!buy && (price >= _follow_slippage))
        price -= _follow_slippage;

    // Create a limit order with the calculated price
    Matching::Order order = Matching::Order::Limit(_next_order_id++, symbol.Id, buy ? Matching::OrderSide::BUY : Matching::OrderSide::SELL, price, _follow_quantity);

    // Add the order to the market manager
    _market_manager.AddOrder(order);
}

} // namespace Execution
} // namespace CppTrader
