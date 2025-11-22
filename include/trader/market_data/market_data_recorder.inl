/*!
    \file market_data_recorder.inl
    rief Market data recorder inline implementation
    \author Ivan Shynkarenka
    \date 2024.XX.XX
    \copyright MIT License
*/

namespace CppTrader {
namespace MarketData {

inline MarketDataRecorder::MarketDataRecorder() = default;

inline MarketDataRecorder::MarketDataRecorder(const std::string& filename)
{
    Open(filename);
}

inline MarketDataRecorder::~MarketDataRecorder() noexcept
{
    Close();
}

inline bool MarketDataRecorder::Open(const std::string& filename)
{
    if (IsOpened())
        return false;

    _filename = filename;
    _file.open(filename, std::ios::binary | std::ios::trunc);
    return IsOpened();
}

inline bool MarketDataRecorder::Close()
{
    if (!IsOpened())
        return false;

    _file.close();
    return !IsOpened();
}

inline void MarketDataRecorder::WriteStart()
{
    if (!IsOpened())
        return;

    char type = 'S';
    Write(type);
}

inline void MarketDataRecorder::WriteStop()
{
    if (!IsOpened())
        return;

    char type = 'E';
    Write(type);
}

inline void MarketDataRecorder::WriteSymbol(const Matching::Symbol& symbol)
{
    if (!IsOpened())
        return;

    char type = 'Y';
    Write(type);
    Write(symbol);
}

inline void MarketDataRecorder::WriteOrderBookUpdate(const Matching::Symbol& symbol, const Matching::LevelUpdate& update)
{
    if (!IsOpened())
        return;

    char type = 'U';
    Write(type);
    Write(symbol.Id);
    Write(update);
}

inline void MarketDataRecorder::WriteTrade(const Matching::Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp)
{
    if (!IsOpened())
        return;

    char type = 'T';
    Write(type);
    Write(symbol.Id);
    Write(price);
    Write(quantity);
    Write(timestamp);
}

inline void MarketDataRecorder::WriteOrder(const Matching::Symbol& symbol, const Matching::Order& order)
{
    if (!IsOpened())
        return;

    char type = 'O';
    Write(type);
    Write(symbol.Id);
    Write(order);
}

inline void MarketDataRecorder::WriteExecution(const Matching::Symbol& symbol, const Matching::Order& order, uint64_t price, uint64_t quantity, uint64_t timestamp)
{
    if (!IsOpened())
        return;

    char type = 'X';
    Write(type);
    Write(symbol.Id);
    Write(order);
    Write(price);
    Write(quantity);
    Write(timestamp);
}

inline void MarketDataRecorder::WriteError(const std::string& message)
{
    if (!IsOpened())
        return;

    char type = '!';
    Write(type);
    WriteString(message);
}

inline void MarketDataRecorder::onMarketDataStart()
{
    WriteStart();
}

inline void MarketDataRecorder::onMarketDataStop()
{
    WriteStop();
}

inline void MarketDataRecorder::onMarketDataSymbol(const Matching::Symbol& symbol)
{
    WriteSymbol(symbol);
}

inline void MarketDataRecorder::onMarketDataOrderBookUpdate(const Matching::Symbol& symbol, const Matching::LevelUpdate& update)
{
    WriteOrderBookUpdate(symbol, update);
}

inline void MarketDataRecorder::onMarketDataTrade(const Matching::Symbol& symbol, uint64_t price, uint64_t quantity, uint64_t timestamp)
{
    WriteTrade(symbol, price, quantity, timestamp);
}

inline void MarketDataRecorder::onMarketDataOrder(const Matching::Symbol& symbol, const Matching::Order& order)
{
    WriteOrder(symbol, order);
}

inline void MarketDataRecorder::onMarketDataExecution(const Matching::Symbol& symbol, const Matching::Order& order, uint64_t price, uint64_t quantity, uint64_t timestamp)
{
    WriteExecution(symbol, order, price, quantity, timestamp);
}

inline void MarketDataRecorder::onMarketDataError(const std::string& message)
{
    WriteError(message);
}

} // namespace MarketData
} // namespace CppTrader
