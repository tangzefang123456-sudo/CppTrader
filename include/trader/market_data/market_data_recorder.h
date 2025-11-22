/*!
    \file market_data_recorder.h
    rief Market data recorder class
    \author Ivan Shynkarenka
    \date 2024.XX.XX
    \copyright MIT License
*/

#ifndef CPPTRADER_MARKET_DATA_MARKET_DATA_RECORDER_H
#define CPPTRADER_MARKET_DATA_MARKET_DATA_RECORDER_H

#include "trader/market_data/market_data_handler.h"
#include "trader/matching/order.h"
#include "trader/matching/level.h"

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace CppTrader {
namespace MarketData {

//! Market data recorder class
/*!
    Market data recorder is used to record market data messages to a binary file.
    It can be used to replay market data later for testing and debugging purposes.

    Not thread-safe.
*/
class MarketDataRecorder : public MarketDataHandler::Handler
{
public:
    MarketDataRecorder();
    MarketDataRecorder(const std::string& filename);
    MarketDataRecorder(const MarketDataRecorder&) noexcept = delete;
    MarketDataRecorder(MarketDataRecorder&&) noexcept = delete;
    virtual ~MarketDataRecorder() noexcept;

    MarketDataRecorder& operator=(const MarketDataRecorder&) noexcept = delete;
    MarketDataRecorder& operator=(MarketDataRecorder&&) noexcept = delete;

    //! Get the recorder filename
    const std::string& filename() const noexcept { return _filename; }

    //! Is the recorder opened?
    bool IsOpened() const noexcept { return _file.is_open(); }

    //! Open the recorder with the given filename
    /*!
        \param filename - Recorder filename
        eturn 'true' if the recorder was successfully opened, 'false' if the recorder failed to open
    */
    bool Open(const std::string& filename);
    //! Close the recorder
    /*!
        eturn 'true' if the recorder was successfully closed, 'false' if the recorder failed to close
    */
    bool Close();

    //! Write market data start event
    void WriteStart();
    //! Write market data stop event
    void WriteStop();

    //! Write market data symbol event
    /*!
        \param symbol - Symbol
    */
    void WriteSymbol(const Matching::Symbol& symbol);

    //! Write market data order book update event
    /*!
        \param symbol - Symbol
        \param update - Price level update
    */
    void WriteOrderBookUpdate(const Matching::Symbol& symbol, const Matching::LevelUpdate& update);

    //! Write market data trade event
    /*!
        \param symbol - Symbol
        \param price - Trade price
        \param quantity - Trade quantity
        \param timestamp - Trade timestamp
    */
    void WriteTrade(const Matching::Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp);

    //! Write market data order event
    /*!
        \param symbol - Symbol
        \param order - Order
    */
    void WriteOrder(const Matching::Symbol& symbol, const Matching::Order& order);

    //! Write market data execution event
    /*!
        \param symbol - Symbol
        \param order - Order
        \param price - Execution price
        \param quantity - Execution quantity
        \param timestamp - Execution timestamp
    */
    void WriteExecution(const Matching::Symbol& symbol, const Matching::Order& order, uint64_t price, uint64_t quantity, uint64_t timestamp);

    //! Write market data error event
    /*!
        \param message - Error message
    */
    void WriteError(const std::string& message);

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
    std::string _filename;
    std::ofstream _file;

    template <typename T>
    void Write(const T& data);
    void WriteString(const std::string& str);
};

} // namespace MarketData
} // namespace CppTrader

#include "market_data_recorder.inl"

#endif // CPPTRADER_MARKET_DATA_MARKET_DATA_RECORDER_H
