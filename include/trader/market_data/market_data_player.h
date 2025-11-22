/*!
    \file market_data_player.h
    rief Market data player class
    \author Ivan Shynkarenka
    \date 2024.XX.XX
    \copyright MIT License
*/

#ifndef CPPTRADER_MARKET_DATA_MARKET_DATA_PLAYER_H
#define CPPTRADER_MARKET_DATA_MARKET_DATA_PLAYER_H

#include "trader/market_data/market_data_handler.h"
#include "trader/matching/market_manager.h"

#include <cstdint>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

namespace CppTrader {
namespace MarketData {

//! Market data player class
/*!
    Market data player is used to play market data messages from a binary file.
    It can be used to replay market data for testing and debugging purposes.

    Not thread-safe.
*/
class MarketDataPlayer : public MarketDataHandler
{
public:
    MarketDataPlayer() noexcept = default;
    MarketDataPlayer(const std::string& filename);
    MarketDataPlayer(const MarketDataPlayer&) noexcept = delete;
    MarketDataPlayer(MarketDataPlayer&&) noexcept = delete;
    virtual ~MarketDataPlayer() noexcept;

    MarketDataPlayer& operator=(const MarketDataPlayer&) noexcept = delete;
    MarketDataPlayer& operator=(MarketDataPlayer&&) noexcept = delete;

    //! Get the player filename
    const std::string& filename() const noexcept { return _filename; }
    //! Get the player speed
    double speed() const noexcept { return _speed; }
    //! Get the player progress
    double progress() const noexcept { return _progress; }

    //! Set the player speed
    /*!
        \param speed - Player speed (1.0 - normal speed, 2.0 - double speed, etc.)
    */
    void SetSpeed(double speed) noexcept { _speed = speed; }

    //! Is the player opened?
    bool IsOpened() const noexcept { return _file.is_open(); }
    //! Is the player playing?
    bool IsPlaying() const noexcept { return _playing; }

    //! Open the player with the given filename
    /*!
        \param filename - Player filename
        eturn 'true' if the player was successfully opened, 'false' if the player failed to open
    */
    bool Open(const std::string& filename);
    //! Close the player
    /*!
        eturn 'true' if the player was successfully closed, 'false' if the player failed to close
    */
    bool Close();

    //! Play the market data
    /*!
        eturn 'true' if the market data was successfully played, 'false' if the market data failed to play
    */
    bool Play();
    //! Pause the market data
    /*!
        eturn 'true' if the market data was successfully paused, 'false' if the market data failed to pause
    */
    bool Pause();
    //! Stop the market data
    /*!
        eturn 'true' if the market data was successfully stopped, 'false' if the market data failed to stop
    */
    bool Stop();

    //! Rewind the market data to the beginning
    /*!
        eturn 'true' if the market data was successfully rewound, 'false' if the market data failed to rewind
    */
    bool Rewind();
    //! Fast forward the market data by the given offset
    /*!
        \param offset - Offset in milliseconds
        eturn 'true' if the market data was successfully fast forwarded, 'false' if the market data failed to fast forward
    */
    bool FastForward(uint64_t offset);
    //! Seek the market data to the given timestamp
    /*!
        \param timestamp - Timestamp
        eturn 'true' if the market data was successfully seeked, 'false' if the market data failed to seek
    */
    bool Seek(uint64_t timestamp);

    // Market data handler implementation
    bool Start() override;
    bool Stop() override;
    bool Restart() override;\n    bool IsRunning() const noexcept override { return IsPlaying(); }

private:
    std::string _filename;
    std::ifstream _file;
    bool _playing{false};
    double _speed{1.0};
    double _progress{0.0};
    uint64_t _start_time{0};
    uint64_t _current_time{0};
    uint64_t _last_time{0};
    std::thread _thread;

    void PlayThread();
    bool ReadNextMessage();

    template <typename T>
    bool Read(T& data);
    bool ReadString(std::string& str);
};

} // namespace MarketData
} // namespace CppTrader

#include "market_data_player.inl"

#endif // CPPTRADER_MARKET_DATA_MARKET_DATA_PLAYER_H
