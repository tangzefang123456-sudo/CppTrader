/*!
    \file market_data_player.cpp
    rief Market data player implementation
    \author Ivan Shynkarenka
    \date 2024.XX.XX
    \copyright MIT License
*/

#include "trader/market_data/market_data_player.h"
#include "trader/matching/symbol.h"

#include <chrono>
#include <thread>

namespace CppTrader {
namespace MarketData {

void MarketDataPlayer::PlayThread()
{
    if (!IsOpened() || !_playing)
        return;

    // Read the first message to get the start time
    if (!ReadNextMessage())
    {
        _playing = false;
        return;
    }

    _start_time = CppCommon::Timestamp::Now().timestamp();

    // Play messages until stopped
    while (_playing && ReadNextMessage())
    {
        // Calculate the elapsed time since the start
        uint64_t current_time = CppCommon::Timestamp::Now().timestamp();
        uint64_t elapsed_time = current_time - _start_time;

        // Calculate the expected time for the current message
        uint64_t expected_time = _current_time - _last_time;

        // If the expected time is greater than the elapsed time, sleep for the difference
        if (expected_time > elapsed_time)
        {
            uint64_t sleep_time = (expected_time - elapsed_time) / 1000000; // Convert to milliseconds
            if (sleep_time > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        }
    }

    _playing = false;
}

bool MarketDataPlayer::ReadNextMessage()
{
    if (!IsOpened() || _file.eof())
        return false;

    // Read the message type
    char type;
    if (!Read(type))
        return false;

    // Process the message based on its type
    switch (type)
    {
        case 'S': // Start message
            if (_handler)
                _handler->onMarketDataStart();
            break;

        case 'E': // Stop message
            if (_handler)
                _handler->onMarketDataStop();
            break;

        case 'Y': // Symbol message
        {
            Matching::Symbol symbol;
            if (!Read(symbol))
                return false;
            if (_handler)
                _handler->onMarketDataSymbol(symbol);
            break;
        }

        case 'U': // Order book update message
        {
            uint32_t symbol_id;
            if (!Read(symbol_id))
                return false;

            // Find the symbol by id
            // This is a placeholder - in a real implementation, we would have a symbol map
            Matching::Symbol symbol(symbol_id, "");

            Matching::LevelUpdate update;
            if (!Read(update))
                return false;
            if (_handler)
                _handler->onMarketDataOrderBookUpdate(symbol, update);
            break;
        }

        case 'T': // Trade message
        {
            uint32_t symbol_id;
            if (!Read(symbol_id))
                return false;

            // Find the symbol by id
            // This is a placeholder - in a real implementation, we would have a symbol map
            Matching::Symbol symbol(symbol_id, "");

            uint64_t price;
            uint64_t quantity;
            uint64_t timestamp;
            if (!Read(price) || !Read(quantity) || !Read(timestamp))
                return false;
            if (_handler)
                _handler->onMarketDataTrade(symbol, price, quantity, timestamp);
            break;
        }

        case 'O': // Order message
        {
            uint32_t symbol_id;
            if (!Read(symbol_id))
                return false;

            // Find the symbol by id
            // This is a placeholder - in a real implementation, we would have a symbol map
            Matching::Symbol symbol(symbol_id, "");

            Matching::Order order;
            if (!Read(order))
                return false;
            if (_handler)
                _handler->onMarketDataOrder(symbol, order);
            break;
        }

        case 'X': // Execution message
        {
            uint32_t symbol_id;
            if (!Read(symbol_id))
                return false;

            // Find the symbol by id
            // This is a placeholder - in a real implementation, we would have a symbol map
            Matching::Symbol symbol(symbol_id, "");

            Matching::Order order;
            uint64_t price;
            uint64_t quantity;
            uint64_t timestamp;
            if (!Read(order) || !Read(price) || !Read(quantity) || !Read(timestamp))
                return false;
            if (_handler)
                _handler->onMarketDataExecution(symbol, order, price, quantity, timestamp);
            break;
        }

        case '!': // Error message
        {
            std::string message;
            if (!ReadString(message))
                return false;
            if (_handler)
                _handler->onMarketDataError(message);
            break;
        }

        default: // Unknown message type
            return false;
    }

    // Update progress
    std::streampos current_pos = _file.tellg();
    std::streampos end_pos = _file.seekg(0, std::ios::end).tellg();
    _file.seekg(current_pos, std::ios::beg);
    _progress = (end_pos > 0) ? (static_cast<double>(current_pos) / static_cast<double>(end_pos)) * 100.0 : 0.0;

    return true;
}

// Template implementation of read operations

template <typename T>
bool MarketDataPlayer::Read(T& data)
{
    if (!IsOpened() || _file.eof())
        return false;

    _file.read(reinterpret_cast<char*>(&data), sizeof(T));
    return _file.good();
}

bool MarketDataPlayer::ReadString(std::string& str)
{
    if (!IsOpened() || _file.eof())
        return false;

    uint32_t length;
    if (!Read(length))
        return false;

    str.resize(length);
    if (length > 0)
    {
        _file.read(&str[0], length);
        if (!_file.good())
            return false;
    }

    return true;
}

// Explicit template instantiation

template bool MarketDataPlayer::Read<bool>(bool&);
template bool MarketDataPlayer::Read<char>(char&);
template bool MarketDataPlayer::Read<uint8_t>(uint8_t&);
template bool MarketDataPlayer::Read<uint16_t>(uint16_t&);
template bool MarketDataPlayer::Read<uint32_t>(uint32_t&);
template bool MarketDataPlayer::Read<uint64_t>(uint64_t&);
template bool MarketDataPlayer::Read<int8_t>(int8_t&);
template bool MarketDataPlayer::Read<int16_t>(int16_t&);
template bool MarketDataPlayer::Read<int32_t>(int32_t&);
template bool MarketDataPlayer::Read<int64_t>(int64_t&);
template bool MarketDataPlayer::Read<float>(float&);
template bool MarketDataPlayer::Read<double>(double&);
template bool MarketDataPlayer::Read<long double>(long double&);
template bool MarketDataPlayer::Read<Matching::Symbol>(Matching::Symbol&);
template bool MarketDataPlayer::Read<Matching::Order>(Matching::Order&);
template bool MarketDataPlayer::Read<Matching::Level>(Matching::Level&);
template bool MarketDataPlayer::Read<Matching::LevelUpdate>(Matching::LevelUpdate&);
template bool MarketDataPlayer::Read<Matching::OrderSide>(Matching::OrderSide&);
template bool MarketDataPlayer::Read<Matching::OrderType>(Matching::OrderType&);
template bool MarketDataPlayer::Read<Matching::OrderTimeInForce>(Matching::OrderTimeInForce&);
template bool MarketDataPlayer::Read<Matching::LevelType>(Matching::LevelType&);
template bool MarketDataPlayer::Read<Matching::UpdateType>(Matching::UpdateType&);

} // namespace MarketData
} // namespace CppTrader
