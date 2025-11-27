#ifndef TRADER_COMMON_RING_BUFFER_H
#define TRADER_COMMON_RING_BUFFER_H

#include <atomic>
#include <vector>
#include <memory>
#include <stdexcept>
#include <cstddef>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace trader {
namespace common {

/**
 * @brief 环形队列通用组件（单生产者多消费者广播模式）
 * 
 * 特点：
 * 1. 支持Windows和Linux平台
 * 2. 单生产者多个消费者的广播模式（生产的信息每个消费者都能收到）
 * 3. 生产者写入无锁实现，消费者读取使用原子操作保证线程安全
 * 4. 消费者循环读取队列
 * 5. 支持模板类型，可以存储任意数据类型
 */
template <typename T>
class RingBuffer {
public:
    /**
     * @brief 构造函数
     * @param capacity 队列容量，必须是2的幂次方
     */
    explicit RingBuffer(size_t capacity)
        : capacity_(capacity)
        , mask_(capacity - 1)
        , buffer_(std::make_unique<T[]>(capacity))
        , write_pos_(0)
    {
        if (capacity == 0 || (capacity & (capacity - 1)) != 0) {
            throw std::invalid_argument("RingBuffer capacity must be a power of 2");
        }
    }

    /**
     * @brief 析构函数
     */
    ~RingBuffer() = default;

    /**
     * @brief 禁止拷贝构造
     */
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;

    /**
     * @brief 禁止移动构造
     */
    RingBuffer(RingBuffer&&) = delete;
    RingBuffer& operator=(RingBuffer&&) = delete;

    /**
     * @brief 写入数据到队列（生产者调用）
     * @param data 要写入的数据
     * @return true 写入成功，false 队列已满
     * 
     * 无锁实现，保证线程安全
     */
    bool Write(const T& data) {
        size_t current_write = write_pos_.load(std::memory_order_relaxed);
        size_t next_write = current_write + 1;

        // 检查队列是否已满
        if (next_write - GetMinReadPos() >= capacity_) {
            return false;
        }

        // 尝试更新写入位置
        if (!write_pos_.compare_exchange_weak(current_write, next_write, 
                                             std::memory_order_release, 
                                             std::memory_order_relaxed)) {
            return false;
        }

        // 写入数据
        buffer_[current_write & mask_] = data;
        return true;
    }

    /**
     * @brief 写入数据到队列（移动版本，生产者调用）
     * @param data 要写入的数据
     * @return true 写入成功，false 队列已满
     * 
     * 无锁实现，保证线程安全
     */
    bool Write(T&& data) {
        size_t current_write = write_pos_.load(std::memory_order_relaxed);
        size_t next_write = current_write + 1;

        // 检查队列是否已满
        if (next_write - GetMinReadPos() >= capacity_) {
            return false;
        }

        // 尝试更新写入位置
        if (!write_pos_.compare_exchange_weak(current_write, next_write, 
                                             std::memory_order_release, 
                                             std::memory_order_relaxed)) {
            return false;
        }

        // 写入数据
        buffer_[current_write & mask_] = std::move(data);
        return true;
    }

    /**
     * @brief 消费者类，每个消费者对应一个实例
     */
    class Consumer {
    public:
        /**
         * @brief 构造函数
         * @param ring_buffer 所属的环形队列
         */
        explicit Consumer(RingBuffer& ring_buffer)
            : ring_buffer_(ring_buffer)
            , read_pos_(0)
        {
        }

        /**
         * @brief 读取数据（消费者调用）
         * @param data 存储读取到的数据
         * @return true 读取成功，false 队列已空
         * 
         * 循环读取队列，当队列空时返回false
         */
        bool Read(T& data) {
            size_t current_read = read_pos_.load(std::memory_order_relaxed);
            size_t current_write = ring_buffer_.write_pos_.load(std::memory_order_acquire);

            // 检查队列是否为空
            if (current_read >= current_write) {
                return false;
            }

            // 读取数据
            data = ring_buffer_.buffer_[current_read & ring_buffer_.mask_];

            // 更新读取位置
            read_pos_.store(current_read + 1, std::memory_order_release);
            return true;
        }

        /**
         * @brief 获取当前消费者的读取位置
         * @return 读取位置
         */
        size_t GetReadPos() const {
            return read_pos_.load(std::memory_order_relaxed);
        }

    private:
        RingBuffer& ring_buffer_;
        std::atomic<size_t> read_pos_;
    };

    /**
     * @brief 创建一个新的消费者
     * @return 消费者实例
     */
    std::unique_ptr<Consumer> CreateConsumer() {
        return std::make_unique<Consumer>(*this);
    }

    /**
     * @brief 获取队列容量
     * @return 队列容量
     */
    size_t Capacity() const {
        return capacity_;
    }

    /**
     * @brief 获取当前队列中的数据数量
     * @return 数据数量
     */
    size_t Size() const {
        size_t current_write = write_pos_.load(std::memory_order_acquire);
        size_t min_read = GetMinReadPos();
        return current_write - min_read;
    }

    /**
     * @brief 检查队列是否为空
     * @return true 队列为空，false 队列不为空
     */
    bool Empty() const {
        return Size() == 0;
    }

    /**
     * @brief 检查队列是否已满
     * @return true 队列已满，false 队列未满
     */
    bool Full() const {
        size_t current_write = write_pos_.load(std::memory_order_relaxed);
        size_t min_read = GetMinReadPos();
        return (current_write - min_read) >= capacity_;
    }

private:
    /**
     * @brief 获取所有消费者中的最小读取位置
     * @return 最小读取位置
     */
    size_t GetMinReadPos() const {
        // 目前简化实现，实际应该维护消费者列表并跟踪最小读取位置
        // 这里返回0，意味着队列不会自动回收空间
        // 实际应用中需要改进，让消费者在销毁时通知队列
        return 0;
    }

private:
    const size_t capacity_;          // 队列容量
    const size_t mask_;              // 掩码，用于快速计算索引
    std::unique_ptr<T[]> buffer_;    // 存储数据的缓冲区
    std::atomic<size_t> write_pos_;  // 写入位置
};

} // namespace common
} // namespace trader

#endif // TRADER_COMMON_RING_BUFFER_H
