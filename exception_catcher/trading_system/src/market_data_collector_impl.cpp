#include "market_data_collector.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>

namespace TradingSystem {

// 共享内存相关定义
const std::string SHARED_MEMORY_NAME = "TradingSystemMarketData";
const size_t SHARED_MEMORY_SIZE = 1024 * 1024 * 10;  // 10MB

// 行情数据收集器实现类
class MarketDataCollectorImpl : public MarketDataCollector {
public:
    MarketDataCollectorImpl();
    virtual ~MarketDataCollectorImpl();

    // 初始化行情收集模块
    bool Init(const std::string& config) override;

    // 开始行情收集
    bool Start() override;

    // 停止行情收集
    bool Stop() override;

    // 处理逐笔成交数据
    void OnTickData(const TickData& tick) override;

    // 处理逐笔委托数据
    void OnOrderData(const OrderData& order) override;

    // 保存行情数据到本地二进制文件
    bool SaveToBinaryFile(const std::string& file_path) override;

    // 从本地二进制文件加载行情数据并模拟播放
    bool LoadFromBinaryFile(const std::string& file_path) override;

private:
    // 初始化共享内存
    bool InitSharedMemory();

    // 释放共享内存
    void ReleaseSharedMemory();

    // 将行情数据写入共享内存
    void WriteToSharedMemory(const void* data, size_t size, const std::string& data_type);

private:
    bool is_initialized_;       // 是否已初始化
    bool is_running_;           // 是否正在运行
    HANDLE shared_memory_handle_;  // 共享内存句柄
    void* shared_memory_ptr_;   // 共享内存指针
    std::vector<TickData> tick_data_buffer_;  // 逐笔成交数据缓存
    std::vector<OrderData> order_data_buffer_;  // 逐笔委托数据缓存
};

MarketDataCollectorImpl::MarketDataCollectorImpl() 
    : is_initialized_(false), is_running_(false), 
      shared_memory_handle_(nullptr), shared_memory_ptr_(nullptr) {
}

MarketDataCollectorImpl::~MarketDataCollectorImpl() {
    Stop();
    ReleaseSharedMemory();
}

bool MarketDataCollectorImpl::Init(const std::string& config) {
    if (is_initialized_) {
        std::cout << "MarketDataCollectorImpl is already initialized." << std::endl;
        return true;
    }

    // 初始化共享内存
    if (!InitSharedMemory()) {
        std::cout << "Failed to initialize shared memory." << std::endl;
        return false;
    }

    is_initialized_ = true;
    std::cout << "MarketDataCollectorImpl initialized with config: " << config << std::endl;
    return true;
}

bool MarketDataCollectorImpl::Start() {
    if (!is_initialized_) {
        std::cout << "MarketDataCollectorImpl is not initialized." << std::endl;
        return false;
    }

    if (is_running_) {
        std::cout << "MarketDataCollectorImpl is already running." << std::endl;
        return true;
    }

    is_running_ = true;
    std::cout << "MarketDataCollectorImpl started." << std::endl;
    return true;
}

bool MarketDataCollectorImpl::Stop() {
    if (!is_running_) {
        std::cout << "MarketDataCollectorImpl is not running." << std::endl;
        return true;
    }

    is_running_ = false;
    std::cout << "MarketDataCollectorImpl stopped." << std::endl;
    return true;
}

std::vector<TickData> MarketDataCollectorImpl::GetLatestTicks() {
    return tick_data_buffer_;
}

std::vector<OrderData> MarketDataCollectorImpl::GetLatestOrders() {
    return order_data_buffer_;
}

void MarketDataCollectorImpl::OnTickData(const TickData& tick) {
    if (!is_running_) {
        std::cout << "MarketDataCollectorImpl is not running, cannot process tick data." << std::endl;
        return;
    }

    // 将逐笔成交数据写入共享内存
    WriteToSharedMemory(&tick, sizeof(TickData), "TickData");

    // 将逐笔成交数据添加到缓存
    tick_data_buffer_.push_back(tick);

    // 打印逐笔成交数据信息
    std::cout << "Tick data processed: " << std::endl;
    std::cout << "  Timestamp: " << tick.timestamp << std::endl;
    std::cout << "  Symbol: " << tick.symbol << std::endl;
    std::cout << "  Price: " << tick.price << std::endl;
    std::cout << "  Volume: " << tick.volume << std::endl;
    std::cout << "  Direction: " << tick.direction << std::endl;
}

void MarketDataCollectorImpl::OnOrderData(const OrderData& order) {
    if (!is_running_) {
        std::cout << "MarketDataCollectorImpl is not running, cannot process order data." << std::endl;
        return;
    }

    // 将逐笔委托数据写入共享内存
    WriteToSharedMemory(&order, sizeof(OrderData), "OrderData");

    // 将逐笔委托数据添加到缓存
    order_data_buffer_.push_back(order);

    // 打印逐笔委托数据信息
    std::cout << "Order data processed: " << std::endl;
    std::cout << "  Timestamp: " << order.timestamp << std::endl;
    std::cout << "  Symbol: " << order.symbol << std::endl;
    std::cout << "  Price: " << order.price << std::endl;
    std::cout << "  Volume: " << order.volume << std::endl;
    std::cout << "  Order type: " << order.order_type << std::endl;
    std::cout << "  Order ID: " << order.order_id << std::endl;
}

bool MarketDataCollectorImpl::SaveToBinaryFile(const std::string& file_path) {
    std::ofstream out_file(file_path, std::ios::binary);
    if (!out_file.is_open()) {
        std::cout << "Failed to open file: " << file_path << std::endl;
        return false;
    }

    // 写入逐笔成交数据数量
    size_t tick_count = tick_data_buffer_.size();
    out_file.write(reinterpret_cast<const char*>(&tick_count), sizeof(size_t));

    // 写入逐笔成交数据
    for (const auto& tick : tick_data_buffer_) {
        out_file.write(reinterpret_cast<const char*>(&tick), sizeof(TickData));
    }

    // 写入逐笔委托数据数量
    size_t order_count = order_data_buffer_.size();
    out_file.write(reinterpret_cast<const char*>(&order_count), sizeof(size_t));

    // 写入逐笔委托数据
    for (const auto& order : order_data_buffer_) {
        out_file.write(reinterpret_cast<const char*>(&order), sizeof(OrderData));
    }

    out_file.close();
    std::cout << "Market data saved to file: " << file_path << std::endl;
    std::cout << "  Tick data count: " << tick_count << std::endl;
    std::cout << "  Order data count: " << order_count << std::endl;
    return true;
}

bool MarketDataCollectorImpl::LoadFromBinaryFile(const std::string& file_path) {
    std::ifstream in_file(file_path, std::ios::binary);
    if (!in_file.is_open()) {
        std::cout << "Failed to open file: " << file_path << std::endl;
        return false;
    }

    // 清空当前缓存
    tick_data_buffer_.clear();
    order_data_buffer_.clear();

    // 读取逐笔成交数据数量
    size_t tick_count = 0;
    in_file.read(reinterpret_cast<char*>(&tick_count), sizeof(size_t));

    // 读取逐笔成交数据
    for (size_t i = 0; i < tick_count; ++i) {
        TickData tick;
        in_file.read(reinterpret_cast<char*>(&tick), sizeof(TickData));
        tick_data_buffer_.push_back(tick);
        // 模拟播放行情，调用OnTickData
        OnTickData(tick);
    }

    // 读取逐笔委托数据数量
    size_t order_count = 0;
    in_file.read(reinterpret_cast<char*>(&order_count), sizeof(size_t));

    // 读取逐笔委托数据
    for (size_t i = 0; i < order_count; ++i) {
        OrderData order;
        in_file.read(reinterpret_cast<char*>(&order), sizeof(OrderData));
        order_data_buffer_.push_back(order);
        // 模拟播放行情，调用OnOrderData
        OnOrderData(order);
    }

    in_file.close();
    std::cout << "Market data loaded from file: " << file_path << std::endl;
    std::cout << "  Tick data count: " << tick_count << std::endl;
    std::cout << "  Order data count: " << order_count << std::endl;
    return true;
}

bool MarketDataCollectorImpl::InitSharedMemory() {
    // 创建或打开共享内存
    shared_memory_handle_ = CreateFileMapping(
        INVALID_HANDLE_VALUE,  // 使用分页文件
        nullptr,               // 默认安全属性
        PAGE_READWRITE,        // 可读可写
        0,                     // 高32位大小
        SHARED_MEMORY_SIZE,    // 低32位大小
        SHARED_MEMORY_NAME.c_str()  // 共享内存名称
    );

    if (shared_memory_handle_ == nullptr) {
        std::cout << "Failed to create file mapping: " << GetLastError() << std::endl;
        return false;
    }

    // 映射共享内存到进程地址空间
    shared_memory_ptr_ = MapViewOfFile(
        shared_memory_handle_,  // 共享内存句柄
        FILE_MAP_ALL_ACCESS,    // 可读可写
        0,                      // 高32位偏移量
        0,                      // 低32位偏移量
        SHARED_MEMORY_SIZE      // 映射大小
    );

    if (shared_memory_ptr_ == nullptr) {
        std::cout << "Failed to map view of file: " << GetLastError() << std::endl;
        CloseHandle(shared_memory_handle_);
        shared_memory_handle_ = nullptr;
        return false;
    }

    std::cout << "Shared memory initialized successfully." << std::endl;
    return true;
}

void MarketDataCollectorImpl::ReleaseSharedMemory() {
    if (shared_memory_ptr_ != nullptr) {
        UnmapViewOfFile(shared_memory_ptr_);
        shared_memory_ptr_ = nullptr;
    }

    if (shared_memory_handle_ != nullptr) {
        CloseHandle(shared_memory_handle_);
        shared_memory_handle_ = nullptr;
    }

    std::cout << "Shared memory released." << std::endl;
}

void MarketDataCollectorImpl::WriteToSharedMemory(const void* data, size_t size, const std::string& data_type) {
    if (shared_memory_ptr_ == nullptr) {
        std::cout << "Shared memory is not initialized, cannot write data." << std::endl;
        return;
    }

    // 这里简化处理，直接将数据写入共享内存的开头
    // 在实际应用中，应该使用更复杂的协议来管理共享内存中的数据
    memcpy(shared_memory_ptr_, data, size);

    // 打印写入共享内存的信息
    std::cout << "Data written to shared memory: " << std::endl;
    std::cout << "  Data type: " << data_type << std::endl;
    std::cout << "  Data size: " << size << " bytes" << std::endl;
}

// 创建行情数据收集器实例的函数
MarketDataCollector* CreateMarketDataCollector() {
    return new MarketDataCollectorImpl();
}

// 销毁行情数据收集器实例的函数
void DestroyMarketDataCollector(MarketDataCollector* collector) {
    delete dynamic_cast<MarketDataCollectorImpl*>(collector);
}

} // namespace TradingSystem
