#pragma once

#include <vector>
#include <atomic>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <type_traits>

#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

namespace common {

/**
 * @brief 环形队列通用组件
 * @tparam T 队列中元素的类型
 * @tparam Size 队列的大小，必须是2的幂
 * 
 * 特性：
 * - 支持单生产者多消费者模式
 * - 生产者写入无锁实现
 * - 消费者循环读取
 * - 广播模式：每个消费者都能收到所有生产的消息
 * - 跨平台支持（Windows/Linux）
 */
template<typename T, size_t Size>
class CircularQueue {
    static_assert((Size & (Size - 1)) == 0, "Size must be a power of two");
    static_assert(std::is_copy_constructible<T>::value || std::is_move_constructible<T>::value, 
                  "T must be copy-constructible or move-constructible");

public:
    CircularQueue() : head_(0), tail_(0) {
        buffer_.resize(Size);
        // 为每个消费者创建独立的读取位置
        consumer_heads_.resize(1); // 默认至少一个消费者
        consumer_heads_[0] = 0;
    }

    ~CircularQueue() = default;

    CircularQueue(const CircularQueue&) = delete;
    CircularQueue& operator=(const CircularQueue&) = delete;

    /**
     * @brief 生产者写入元素（无锁）
     * @param value 要写入的元素
     * @return 成功返回true，队列满返回false
     */
    bool Push(const T& value) {
        return PushImpl(value);
    }

    bool Push(T&& value) {
        return PushImpl(std::move(value));
    }

    /**
     * @brief 消费者读取元素
     * @param value 读取到的元素
     * @param consumer_id 消费者ID（从0开始）
     * @return 成功返回true，队列为空返回false
     */
    bool Pop(T& value, size_t consumer_id = 0) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // 确保消费者ID有效
        if (consumer_id >= consumer_heads_.size()) {
            throw std::out_of_range("Invalid consumer ID");
        }

        size_t head = consumer_heads_[consumer_id];
        size_t tail = tail_.load(std::memory_order_acquire);

        if (head == tail) {
            return false; // 队列为空
        }

        value = std::move(buffer_[head & (Size - 1)]);
        consumer_heads_[consumer_id] = head + 1;
        
        return true;
    }

    /**
     * @brief 消费者阻塞读取元素
     * @param value 读取到的元素
     * @param consumer_id 消费者ID（从0开始）
     * @param timeout_ms 超时时间（毫秒），0表示无限等待
     * @return 成功返回true，超时返回false
     */
    bool PopBlocking(T& value, size_t consumer_id = 0, uint32_t timeout_ms = 0) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // 确保消费者ID有效
        if (consumer_id >= consumer_heads_.size()) {
            throw std::out_of_range("Invalid consumer ID");
        }

        size_t head = consumer_heads_[consumer_id];
        size_t tail = tail_.load(std::memory_order_acquire);

        if (head == tail) {
            if (timeout_ms == 0) {
                cond_var_.wait(lock, [this, consumer_id]() {
                    return consumer_heads_[consumer_id] != tail_.load(std::memory_order_acquire);
                });
            } else {
                if (!cond_var_.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this, consumer_id]() {
                    return consumer_heads_[consumer_id] != tail_.load(std::memory_order_acquire);
                })) {
                    return false; // 超时
                }
            }
        }

        head = consumer_heads_[consumer_id];
        tail = tail_.load(std::memory_order_acquire);
        value = std::move(buffer_[head & (Size - 1)]);
        consumer_heads_[consumer_id] = head + 1;
        
        return true;
    }

    /**
     * @brief 添加消费者
     * @return 新消费者的ID
     */
    size_t AddConsumer() {
        std::unique_lock<std::mutex> lock(mutex_);
        size_t id = consumer_heads_.size();
        consumer_heads_.push_back(tail_.load(std::memory_order_acquire));
        return id;
    }

    /**
     * @brief 获取队列中元素的数量（对于特定消费者）
     * @param consumer_id 消费者ID
     * @return 元素数量
     */
    size_t Size(size_t consumer_id = 0) const {
        std::unique_lock<std::mutex> lock(mutex_);
        
        if (consumer_id >= consumer_heads_.size()) {
            throw std::out_of_range("Invalid consumer ID");
        }

        size_t head = consumer_heads_[consumer_id];
        size_t tail = tail_.load(std::memory_order_acquire);
        return tail - head;
    }

    /**
     * @brief 检查队列是否为空（对于特定消费者）
     * @param consumer_id 消费者ID
     * @return 为空返回true，否则返回false
     */
    bool Empty(size_t consumer_id = 0) const {
        return Size(consumer_id) == 0;
    }

    /**
     * @brief 检查队列是否已满（生产者视角）
     * @return 已满返回true，否则返回false
     */
    bool Full() const {
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tail_.load(std::memory_order_acquire);
        return (tail - head) == Size;
    }

private:
    template<typename U>
    bool PushImpl(U&& value) {
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tail_.load(std::memory_order_acquire);

        if ((tail - head) == Size) {
            return false; // 队列满
        }

        buffer_[tail & (Size - 1)] = std::forward<U>(value);
        tail_.store(tail + 1, std::memory_order_release);

        // 通知等待的消费者
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.notify_all();

        return true;
    }

private:
    std::vector<T> buffer_;
    std::atomic<size_t> head_;    // 生产者头部（所有消费者中最旧的读取位置）
    std::atomic<size_t> tail_;    // 生产者尾部
    
    mutable std::mutex mutex_;
    std::condition_variable cond_var_;
    std::vector<size_t> consumer_heads_; // 每个消费者的独立读取位置
};

} // namespace common
