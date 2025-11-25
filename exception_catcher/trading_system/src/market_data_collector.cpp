#include "market_data_collector.h"
#include <iostream>
#include <cstring>

namespace TradingSystem {

bool MarketDataCollector::SaveToBinaryFile(const std::string& file_path) {
    std::ofstream out_file(file_path, std::ios::binary | std::ios::out);
    if (!out_file.is_open()) {
        std::cerr << "Failed to open binary file for writing: " << file_path << std::endl;
        return false;
    }

    // 写入逐笔成交数据数量
    size_t tick_count = ticks_.size();
    out_file.write(reinterpret_cast<const char*>(&tick_count), sizeof(tick_count));

    // 写入逐笔成交数据
    for (const auto& tick : ticks_) {
        out_file.write(reinterpret_cast<const char*>(&tick.timestamp), sizeof(tick.timestamp));
        
        size_t symbol_length = tick.symbol.size();
        out_file.write(reinterpret_cast<const char*>(&symbol_length), sizeof(symbol_length));
        out_file.write(tick.symbol.c_str(), symbol_length);
        
        out_file.write(reinterpret_cast<const char*>(&tick.price), sizeof(tick.price));
        out_file.write(reinterpret_cast<const char*>(&tick.volume), sizeof(tick.volume));
        out_file.write(&tick.direction, sizeof(tick.direction));
    }

    // 写入逐笔委托数据数量
    size_t order_count = orders_.size();
    out_file.write(reinterpret_cast<const char*>(&order_count), sizeof(order_count));

    // 写入逐笔委托数据
    for (const auto& order : orders_) {
        out_file.write(reinterpret_cast<const char*>(&order.timestamp), sizeof(order.timestamp));
        
        size_t symbol_length = order.symbol.size();
        out_file.write(reinterpret_cast<const char*>(&symbol_length), sizeof(symbol_length));
        out_file.write(order.symbol.c_str(), symbol_length);
        
        out_file.write(&order.order_type, sizeof(order.order_type));
        out_file.write(reinterpret_cast<const char*>(&order.price), sizeof(order.price));
        out_file.write(reinterpret_cast<const char*>(&order.volume), sizeof(order.volume));
        out_file.write(reinterpret_cast<const char*>(&order.order_id), sizeof(order.order_id));
    }

    out_file.close();
    std::cout << "Successfully saved market data to binary file: " << file_path << std::endl;
    return true;
}

bool MarketDataCollector::LoadFromBinaryFile(const std::string& file_path) {
    std::ifstream in_file(file_path, std::ios::binary | std::ios::in);
    if (!in_file.is_open()) {
        std::cerr << "Failed to open binary file for reading: " << file_path << std::endl;
        return false;
    }

    // 清空现有数据
    ticks_.clear();
    orders_.clear();

    // 读取逐笔成交数据数量
    size_t tick_count = 0;
    in_file.read(reinterpret_cast<char*>(&tick_count), sizeof(tick_count));

    // 读取逐笔成交数据
    for (size_t i = 0; i < tick_count; ++i) {
        TickData tick;
        
        in_file.read(reinterpret_cast<char*>(&tick.timestamp), sizeof(tick.timestamp));
        
        size_t symbol_length = 0;
        in_file.read(reinterpret_cast<char*>(&symbol_length), sizeof(symbol_length));
        tick.symbol.resize(symbol_length);
        in_file.read(&tick.symbol[0], symbol_length);
        
        in_file.read(reinterpret_cast<char*>(&tick.price), sizeof(tick.price));
        in_file.read(reinterpret_cast<char*>(&tick.volume), sizeof(tick.volume));
        in_file.read(&tick.direction, sizeof(tick.direction));
        
        ticks_.push_back(tick);
    }

    // 读取逐笔委托数据数量
    size_t order_count = 0;
    in_file.read(reinterpret_cast<char*>(&order_count), sizeof(order_count));

    // 读取逐笔委托数据
    for (size_t i = 0; i < order_count; ++i) {
        OrderData order;
        
        in_file.read(reinterpret_cast<char*>(&order.timestamp), sizeof(order.timestamp));
        
        size_t symbol_length = 0;
        in_file.read(reinterpret_cast<char*>(&symbol_length), sizeof(symbol_length));
        order.symbol.resize(symbol_length);
        in_file.read(&order.symbol[0], symbol_length);
        
        in_file.read(&order.order_type, sizeof(order.order_type));
        in_file.read(reinterpret_cast<char*>(&order.price), sizeof(order.price));
        in_file.read(reinterpret_cast<char*>(&order.volume), sizeof(order.volume));
        in_file.read(reinterpret_cast<char*>(&order.order_id), sizeof(order.order_id));
        
        orders_.push_back(order);
    }

    in_file.close();
    std::cout << "Successfully loaded market data from binary file: " << file_path << std::endl;
    std::cout << "Loaded " << ticks_.size() << " ticks and " << orders_.size() << " orders." << std::endl;
    return true;
}

} // namespace TradingSystem
