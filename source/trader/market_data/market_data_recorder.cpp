/*!
    \file market_data_recorder.cpp
    rief Market data recorder implementation
    \author Ivan Shynkarenka
    \date 2024.XX.XX
    \copyright MIT License
*/

#include "trader/market_data/market_data_recorder.h"

namespace CppTrader {
namespace MarketData {

// Template implementation of write operations

template <typename T>
void MarketDataRecorder::Write(const T& data)
{
    if (!IsOpened())
        return;

    _file.write(reinterpret_cast<const char*>(&data), sizeof(T));
}

void MarketDataRecorder::WriteString(const std::string& str)
{
    if (!IsOpened())
        return;

    uint32_t length = str.size();
    Write(length);
    _file.write(str.data(), length);
}

// Explicit template instantiation

template void MarketDataRecorder::Write<bool>(const bool&);
template void MarketDataRecorder::Write<char>(const char&);
template void MarketDataRecorder::Write<uint8_t>(const uint8_t&);
template void MarketDataRecorder::Write<uint16_t>(const uint16_t&);
template void MarketDataRecorder::Write<uint32_t>(const uint32_t&);
template void MarketDataRecorder::Write<uint64_t>(const uint64_t&);
template void MarketDataRecorder::Write<int8_t>(const int8_t&);
template void MarketDataRecorder::Write<int16_t>(const int16_t&);
template void MarketDataRecorder::Write<int32_t>(const int32_t&);
template void MarketDataRecorder::Write<int64_t>(const int64_t&);
template void MarketDataRecorder::Write<float>(const float&);
template void MarketDataRecorder::Write<double>(const double&);
template void MarketDataRecorder::Write<long double>(const long double&);
template void MarketDataRecorder::Write<Matching::Symbol>(const Matching::Symbol&);
template void MarketDataRecorder::Write<Matching::Order>(const Matching::Order&);
template void MarketDataRecorder::Write<Matching::Level>(const Matching::Level&);
template void MarketDataRecorder::Write<Matching::LevelUpdate>(const Matching::LevelUpdate&);
template void MarketDataRecorder::Write<Matching::OrderSide>(const Matching::OrderSide&);
template void MarketDataRecorder::Write<Matching::OrderType>(const Matching::OrderType&);
template void MarketDataRecorder::Write<Matching::OrderTimeInForce>(const Matching::OrderTimeInForce&);
template void MarketDataRecorder::Write<Matching::LevelType>(const Matching::LevelType&);
template void MarketDataRecorder::Write<Matching::UpdateType>(const Matching::UpdateType&);

} // namespace MarketData
} // namespace CppTrader
