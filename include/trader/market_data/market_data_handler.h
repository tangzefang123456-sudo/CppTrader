/*!
    \file market_data_handler.h
    rief Market data handler abstract class
    \author Ivan Shynkarenka
    \date 2024.XX.XX
    \copyright MIT License
*/

#ifndef CPPTRADER_MARKET_DATA_MARKET_DATA_HANDLER_H
#define CPPTRADER_MARKET_DATA_MARKET_DATA_HANDLER_H

#include "trader/matching/order.h"
#include "trader/matching/level.h"

#include <cstdint>
#include <string>

namespace CppTrader {
namespace MarketData {

//! Market data handler abstract class
/*!
    Market data handler is used to handle market data messages from different sources.
    It provides a common interface for all market data handlers.

    Not thread-safe.
*/
class MarketDataHandler
{
public:
    //! Market data handler event handler
    class Handler
    {
    public:
        //! Handle market data start event
        virtual void onMarketDataStart() {}
        //! Handle market data stop event
        virtual void onMarketDataStop() {}

        //! Handle market data symbol event
        /*!
            \param symbol - Symbol
        */
        virtual void onMarketDataSymbol(const Matching::Symbol& symbol) {}

        //! Handle market data order book update event
        /*!
            \param symbol - Symbol
            \param update - Price level update
        */
        virtual void onMarketDataOrderBookUpdate(const Matching::Symbol& symbol, const Matching::LevelUpdate& update) {}

        //! Handle market data trade event
        /*!
            \param symbol - Symbol
            \param price - Trade price
            \param quantity - Trade quantity
            \param timestamp - Trade timestamp
        */
        virtual void onMarketDataTrade(const Matching::Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp) {}

        //! Handle market data order event
        /*!
            \param symbol - Symbol
            \param order - Order
        */
        virtual void onMarketDataOrder(const Matching::Symbol& symbol, const Matching::Order& order) {}

        //! Handle market data execution event
        /*!
            \param symbol - Symbol
            \param order - Order
            \param price - Execution price
            \param quantity - Execution quantity
            \param timestamp - Execution timestamp
        */
        virtual void onMarketDataExecution(const Matching::Symbol& symbol, const Matching::Order& order, uint64_t price, uint64_t quantity, uint64_t timestamp) {}

        //! Handle market data error event
        /*!
            \param message - Error message
        */
        virtual void onMarketDataError(const std::string& message) {}
    };

    MarketDataHandler() noexcept = default;
    MarketDataHandler(const MarketDataHandler&) noexcept = delete;
    MarketDataHandler(MarketDataHandler&&) noexcept = delete;
    virtual ~MarketDataHandler() noexcept = default;

    MarketDataHandler& operator=(const MarketDataHandler&) noexcept = delete;
    MarketDataHandler& operator=(MarketDataHandler&&) noexcept = delete;

    //! Get the market data handler
    Handler* handler() noexcept { return _handler; }
    const Handler* handler() const noexcept { return _handler; }
    //! Set the market data handler
    void SetHandler(Handler* handler) noexcept { _handler = handler; }

    //! Start the market data handler
    /*!
        
eturn 'true' if the market data handler was successfully started, 'false' if the market data handler failed to start
    */
    virtual bool Start() = 0;
    //! Stop the market data handler
    /*!
        
eturn 'true' if the market data handler was successfully stopped, 'false' if the market data handler failed to stop
    */
    virtual bool Stop() = 0;
    //! Restart the market data handler
    /*!
        
eturn 'true' if the market data handler was successfully restarted, 'false' if the market data handler failed to restart
    */
    virtual bool Restart() = 0;

    //! Is the market data handler running?
    virtual bool IsRunning() const noexcept = 0;

protected:
    Handler* _handler{nullptr};
};

} // namespace MarketData
} // namespace CppTrader

#endif // CPPTRADER_MARKET_DATA_MARKET_DATA_HANDLER_H
