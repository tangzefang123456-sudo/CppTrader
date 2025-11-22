/*!
    \file signal_generator.inl
    rief Signal generator inline implementation
    \author Ivan Shynkarenka
    \date 2024.XX.XX
    \copyright MIT License
*/

namespace CppTrader {
namespace Signal {

inline SignalGenerator::SignalGenerator(Matching::MarketManager& market_manager)
    : _market_manager(market_manager)
{
}

inline SignalGenerator::~SignalGenerator() noexcept
{
    Stop();
}

inline bool SignalGenerator::Start()
{
    if (IsRunning())
        return false;

    _running = true;
    if (_handler)
        _handler->onSignalGeneratorStart();
    return true;
}

inline bool SignalGenerator::Stop()
{
    if (!IsRunning())
        return false;

    _running = false;
    if (_handler)
        _handler->onSignalGeneratorStop();
    return true;
}

inline bool SignalGenerator::Restart()
{
    if (IsRunning())
        Stop();
    return Start();
}

inline void SignalGenerator::onMarketDataStart()
{
    Start();
}

inline void SignalGenerator::onMarketDataStop()
{
    Stop();
}

inline void SignalGenerator::onMarketDataSymbol(const Matching::Symbol& symbol)
{
    // Add the symbol to the market manager
    _market_manager.AddSymbol(symbol);

    // Create an order book for the symbol
    _order_books.emplace(symbol.Id, Matching::OrderBook(_market_manager, symbol));

    if (_handler)
        _handler->onSymbolAdded(symbol);
}

inline void SignalGenerator::onMarketDataOrderBookUpdate(const Matching::Symbol& symbol, const Matching::LevelUpdate& update)
{
    // Find the order book for the symbol
    auto it = _order_books.find(symbol.Id);
    if (it == _order_books.end())
        return;

    Matching::OrderBook& order_book = it->second;

    // Update the order book with the new level update
    if (update.Update.IsBid())
    {
        if (update.Type == Matching::UpdateType::ADD)
            order_book.AddLevel(update.Update);
        else if (update.Type == Matching::UpdateType::UPDATE)
            order_book.UpdateLevel(update.Update);
        else if (update.Type == Matching::UpdateType::DELETE)
            order_book.DeleteLevel(update.Update.Price);
    }
    else
    {
        if (update.Type == Matching::UpdateType::ADD)
            order_book.AddLevel(update.Update);
        else if (update.Type == Matching::UpdateType::UPDATE)
            order_book.UpdateLevel(update.Update);
        else if (update.Type == Matching::UpdateType::DELETE)
            order_book.DeleteLevel(update.Update.Price);
    }

    if (_handler)
        _handler->onOrderBookUpdated(symbol, order_book);
}

inline void SignalGenerator::onMarketDataTrade(const Matching::Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp)
{
    // Calculate trade amount
    uint64_t amount = price * quantity;

    // Notify trade event
    if (_handler)
        _handler->onTrade(symbol, price, quantity, timestamp);

    // Check if this is a big trade
    if (amount >= _big_trade_threshold)
    {
        if (_handler)
            _handler->onBigTradeSignal(symbol, price, quantity, timestamp, amount);
    }
}

inline void SignalGenerator::onMarketDataOrder(const Matching::Symbol& symbol, const Matching::Order& order)
{
    // Find the order book for the symbol
    auto it = _order_books.find(symbol.Id);
    if (it == _order_books.end())
        return;

    Matching::OrderBook& order_book = it->second;

    // Add the order to the order book
    order_book.AddOrder(const_cast<Matching::Order*>(&order));

    if (_handler)
        _handler->onOrder(symbol, order);
}

inline void SignalGenerator::onMarketDataExecution(const Matching::Symbol& symbol, const Matching::Order& order, uint64_t price, uint64_t quantity, uint64_t timestamp)
{
    if (_handler)
        _handler->onExecution(symbol, order, price, quantity, timestamp);
}

inline void SignalGenerator::onMarketDataError(const std::string& message)
{
    if (_handler)
        _handler->onError(message);
}

} // namespace Signal
} // namespace CppTrader
