/*!
    \file signal_generator.h
    rief Signal generator class
    \author Ivan Shynkarenka
    \date 2024.XX.XX
    \copyright MIT License
*/

#ifndef CPPTRADER_SIGNAL_SIGNAL_GENERATOR_H
#define CPPTRADER_SIGNAL_SIGNAL_GENERATOR_H

#include "trader/market_data/market_data_handler.h"
#include "trader/matching/order_book.h"
#include "trader/matching/market_manager.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace CppTrader {
namespace Signal {

//! Signal generator event handler
class SignalHandler
{
public:
    //! Handle signal generator start event
    virtual void onSignalGeneratorStart() {}
    //! Handle signal generator stop event
    virtual void onSignalGeneratorStop() {}

    //! Handle symbol added event
    /*!
        \param symbol - Symbol
    */
    virtual void onSymbolAdded(const Matching::Symbol& symbol) {}
    //! Handle symbol removed event
    /*!
        \param symbol - Symbol
    */
    virtual void onSymbolRemoved(const Matching::Symbol& symbol) {}

    //! Handle order book updated event
    /*!
        \param symbol - Symbol
        \param order_book - Order book
    */
    virtual void onOrderBookUpdated(const Matching::Symbol& symbol, const Matching::OrderBook& order_book) {}

    //! Handle trade event
    /*!
        \param symbol - Symbol
        \param price - Trade price
        \param quantity - Trade quantity
        \param timestamp - Trade timestamp
    */
    virtual void onTrade(const Matching::Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp) {}

    //! Handle big trade signal event
    /*!
        \param symbol - Symbol
        \param price - Trade price
        \param quantity - Trade quantity
        \param timestamp - Trade timestamp
        \param amount - Trade amount
    */
    virtual void onBigTradeSignal(const Matching::Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp, uint64_t amount) {}

    //! Handle order event
    /*!
        \param symbol - Symbol
        \param order - Order
    */
    virtual void onOrder(const Matching::Symbol& symbol, const Matching::Order& order) {}

    //! Handle execution event
    /*!
        \param symbol - Symbol
        \param order - Order
        \param price - Execution price
        \param quantity - Execution quantity
        \param timestamp - Execution timestamp
    */
    virtual void onExecution(const Matching::Symbol& symbol, const Matching::Order& order, uint64_t price, uint64_t quantity, uint64_t timestamp) {}

    //! Handle error event
    /*!
        \param message - Error message
    */
    virtual void onError(const std::string& message) {}
};

//! Signal generator class
/*!
    Signal generator is used to generate trading signals based on market data.
    It maintains order books for all symbols and generates big trade signals when
    a trade amount exceeds a certain threshold (default: 100,000).

    Not thread-safe.
*/
class SignalGenerator : public MarketData::MarketDataHandler::Handler
{
public:
    SignalGenerator() noexcept = default;
    SignalGenerator(Matching::MarketManager& market_manager);
    SignalGenerator(const SignalGenerator&) noexcept = delete;
    SignalGenerator(SignalGenerator&&) noexcept = delete;
    virtual ~SignalGenerator() noexcept;

    SignalGenerator& operator=(const SignalGenerator&) noexcept = delete;
    SignalGenerator& operator=(SignalGenerator&&) noexcept = delete;

    //! Get the signal handler
    SignalHandler* handler() noexcept { return _handler; }
    const SignalHandler* handler() const noexcept { return _handler; }
    //! Set the signal handler
    void SetHandler(SignalHandler* handler) noexcept { _handler = handler; }

    //! Get the big trade threshold
    uint64_t big_trade_threshold() const noexcept { return _big_trade_threshold; }
    //! Set the big trade threshold
    void SetBigTradeThreshold(uint64_t threshold) noexcept { _big_trade_threshold = threshold; }

    //! Get the market manager
    Matching::MarketManager& market_manager() noexcept { return _market_manager; }
    const Matching::MarketManager& market_manager() const noexcept { return _market_manager; }

    //! Start the signal generator
    /*!
        eturn 'true' if the signal generator was successfully started, 'false' if the signal generator failed to start
    */
    bool Start();
    //! Stop the signal generator
    /*!
        eturn 'true' if the signal generator was successfully stopped, 'false' if the signal generator failed to stop
    */
    bool Stop();
    //! Restart the signal generator
    /*!
        eturn 'true' if the signal generator was successfully restarted, 'false' if the signal generator failed to restart
    */
    bool Restart();

    //! Is the signal generator running?
    bool IsRunning() const noexcept { return _running; }

protected:
    // Market data handler implementation
    void onMarketDataStart() override;
    void onMarketDataStop() override;
    void onMarketDataSymbol(const Matching::Symbol& symbol) override;
    void onMarketDataOrderBookUpdate(const Matching::Symbol& symbol, const Matching::LevelUpdate& update) override;
    void onMarketDataTrade(const Matching::Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp) override;
    void onMarketDataOrder(const Matching::Symbol& symbol, const Matching::Order& order) override;
    void onMarketDataExecution(const Matching::Symbol& symbol, const Matching::Order& order, uint64_t price, uint64_t quantity, uint64_t timestamp) override;
    void onMarketDataError(const std::string& message) override;

private:
    Matching::MarketManager& _market_manager;
    SignalHandler* _handler{nullptr};
    bool _running{false};
    uint64_t _big_trade_threshold{100000}; // 100,000

    // Order books for all symbols
    std::map<uint32_t, Matching::OrderBook> _order_books;
};

} // namespace Signal
} // namespace CppTrader

#include "signal_generator.inl"

#endif // CPPTRADER_SIGNAL_SIGNAL_GENERATOR_H
