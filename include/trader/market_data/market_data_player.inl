/*!
    \file market_data_player.inl
    rief Market data player inline implementation
    \author Ivan Shynkarenka
    \date 2024.XX.XX
    \copyright MIT License
*/

namespace CppTrader {
namespace MarketData {

inline MarketDataPlayer::MarketDataPlayer(const std::string& filename)
{
    Open(filename);
}

inline MarketDataPlayer::~MarketDataPlayer() noexcept
{
    Stop();
    Close();
}

inline bool MarketDataPlayer::Open(const std::string& filename)
{
    if (IsOpened())
        return false;

    _filename = filename;
    _file.open(filename, std::ios::binary);
    return IsOpened();
}

inline bool MarketDataPlayer::Close()
{
    if (!IsOpened())
        return false;

    _file.close();
    return !IsOpened();
}

inline bool MarketDataPlayer::Play()
{
    if (!IsOpened() || IsPlaying())
        return false;

    _playing = true;
    _thread = std::thread(&MarketDataPlayer::PlayThread, this);
    return true;
}

inline bool MarketDataPlayer::Pause()
{
    if (!IsPlaying())
        return false;

    _playing = false;
    if (_thread.joinable())
        _thread.join();
    return true;
}

inline bool MarketDataPlayer::Stop()
{
    if (!IsPlaying())
        return false;

    _playing = false;
    if (_thread.joinable())
        _thread.join();
    return Rewind();
}

inline bool MarketDataPlayer::Rewind()
{
    if (!IsOpened() || IsPlaying())
        return false;

    _file.seekg(0, std::ios::beg);
    _progress = 0.0;
    _current_time = 0;
    _last_time = 0;
    return true;
}

inline bool MarketDataPlayer::FastForward(uint64_t offset)
{
    if (!IsOpened() || IsPlaying())
        return false;

    // Not implemented yet
    return false;
}

inline bool MarketDataPlayer::Seek(uint64_t timestamp)
{
    if (!IsOpened() || IsPlaying())
        return false;

    // Not implemented yet
    return false;
}

inline bool MarketDataPlayer::Start()
{
    return Play();
}

inline bool MarketDataPlayer::Stop()
{
    return Stop();
}

inline bool MarketDataPlayer::Restart()
{
    if (IsPlaying())
        Stop();
    return Play();
}

} // namespace MarketData
} // namespace CppTrader
