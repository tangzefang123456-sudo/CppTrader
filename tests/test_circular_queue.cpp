#include "test.h"
#include "common/circular_queue.h"
#include <thread>
#include <vector>
#include <atomic>
#include <iostream>

using namespace common;

// 测试基本功能
TEST(CircularQueueTest, BasicFunctionality) {
    CircularQueue<int, 8> queue;
    
    // 测试空队列
    EXPECT_TRUE(queue.Empty());
    EXPECT_EQ(queue.Size(), 0);
    EXPECT_FALSE(queue.Full());
    
    // 测试Push
    int value = 0;
    EXPECT_TRUE(queue.Push(1));
    EXPECT_TRUE(queue.Push(2));
    EXPECT_TRUE(queue.Push(3));
    
    // 测试Size
    EXPECT_EQ(queue.Size(), 3);
    EXPECT_FALSE(queue.Empty());
    EXPECT_FALSE(queue.Full());
    
    // 测试Pop
    EXPECT_TRUE(queue.Pop(value));
    EXPECT_EQ(value, 1);
    EXPECT_EQ(queue.Size(), 2);
    
    EXPECT_TRUE(queue.Pop(value));
    EXPECT_EQ(value, 2);
    EXPECT_EQ(queue.Size(), 1);
    
    EXPECT_TRUE(queue.Pop(value));
    EXPECT_EQ(value, 3);
    EXPECT_EQ(queue.Size(), 0);
    EXPECT_TRUE(queue.Empty());
    
    // 测试队列为空时Pop返回false
    EXPECT_FALSE(queue.Pop(value));
}

// 测试队列满
TEST(CircularQueueTest, QueueFull) {
    CircularQueue<int, 4> queue;
    
    // 填充队列
    EXPECT_TRUE(queue.Push(1));
    EXPECT_TRUE(queue.Push(2));
    EXPECT_TRUE(queue.Push(3));
    EXPECT_TRUE(queue.Push(4));
    
    // 队列应该满了
    EXPECT_TRUE(queue.Full());
    EXPECT_FALSE(queue.Push(5)); // 无法再Push
    
    // 弹出一个元素后可以继续Push
    int value = 0;
    EXPECT_TRUE(queue.Pop(value));
    EXPECT_EQ(value, 1);
    EXPECT_FALSE(queue.Full());
    EXPECT_TRUE(queue.Push(5));
    EXPECT_TRUE(queue.Full());
}

// 测试多消费者广播模式
TEST(CircularQueueTest, MultipleConsumers) {
    const size_t kQueueSize = 16;
    const size_t kNumConsumers = 3;
    const size_t kNumMessages = 10;
    
    CircularQueue<int, kQueueSize> queue;
    std::vector<std::thread> consumers;
    std::vector<std::atomic<size_t>> counters(kNumConsumers);
    std::vector<std::atomic<bool>> done(kNumConsumers);
    
    // 初始化计数器和done标志
    for (size_t i = 0; i < kNumConsumers; ++i) {
        counters[i] = 0;
        done[i] = false;
    }
    
    // 创建消费者线程
    for (size_t i = 0; i < kNumConsumers; ++i) {
        size_t consumer_id = queue.AddConsumer();
        consumers.emplace_back([&, consumer_id, i]() {
            int value = 0;
            while (!done[i] || !queue.Empty(consumer_id)) {
                if (queue.Pop(value, consumer_id)) {
                    counters[i]++;
                    // 验证消息顺序
                    EXPECT_EQ(value, static_cast<int>(counters[i]));
                } else {
                    // 队列为空时短暂睡眠
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                }
            }
        });
    }
    
    // 生产者线程
    std::thread producer([&]() {
        for (int i = 1; i <= static_cast<int>(kNumMessages); ++i) {
            while (!queue.Push(i)) {
                // 队列满时短暂睡眠
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        }
    });
    
    // 等待生产者完成
    producer.join();
    
    // 等待所有消费者处理完消息
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 设置done标志并等待消费者线程结束
    for (auto& d : done) {
        d = true;
    }
    
    for (auto& consumer : consumers) {
        consumer.join();
    }
    
    // 验证每个消费者都收到了所有消息
    for (size_t i = 0; i < kNumConsumers; ++i) {
        EXPECT_EQ(counters[i], kNumMessages);
    }
}

// 测试阻塞读取
TEST(CircularQueueTest, BlockingPop) {
    CircularQueue<int, 8> queue;
    
    int value = 0;
    bool received = false;
    
    // 创建生产者线程，延迟100ms后Push
    std::thread producer([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        queue.Push(42);
    });
    
    // 测试阻塞读取，应该在100ms后收到消息
    auto start = std::chrono::steady_clock::now();
    received = queue.PopBlocking(value, 0, 200); // 200ms超时
    auto end = std::chrono::steady_clock::now();
    
    producer.join();
    
    EXPECT_TRUE(received);
    EXPECT_EQ(value, 42);
    
    // 验证等待时间在合理范围内
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_GE(duration.count(), 100);
    EXPECT_LE(duration.count(), 200);
}

// 测试超时读取
TEST(CircularQueueTest, PopTimeout) {
    CircularQueue<int, 8> queue;
    
    int value = 0;
    
    // 测试超时读取，队列为空，应该超时返回false
    auto start = std::chrono::steady_clock::now();
    bool received = queue.PopBlocking(value, 0, 100); // 100ms超时
    auto end = std::chrono::steady_clock::now();
    
    EXPECT_FALSE(received);
    
    // 验证等待时间在合理范围内
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_GE(duration.count(), 100);
    EXPECT_LE(duration.count(), 150); // 允许50ms误差
}

// 测试移动语义
TEST(CircularQueueTest, MoveSemantics) {
    struct MoveOnly {
        int value;
        MoveOnly(int v) : value(v) {}
        MoveOnly(const MoveOnly&) = delete;
        MoveOnly& operator=(const MoveOnly&) = delete;
        MoveOnly(MoveOnly&& other) noexcept : value(other.value) {
            other.value = -1;
        }
        MoveOnly& operator=(MoveOnly&& other) noexcept {
            if (this != &other) {
                value = other.value;
                other.value = -1;
            }
            return *this;
        }
    };
    
    CircularQueue<MoveOnly, 8> queue;
    
    // 测试移动Push
    EXPECT_TRUE(queue.Push(MoveOnly(42)));
    
    // 测试移动Pop
    MoveOnly mo(0);
    EXPECT_TRUE(queue.Pop(mo));
    EXPECT_EQ(mo.value, 42);
}

// 测试线程安全
TEST(CircularQueueTest, ThreadSafety) {
    const size_t kQueueSize = 32;
    const size_t kNumProducers = 1; // 单生产者
    const size_t kNumConsumers = 2;
    const size_t kNumMessagesPerProducer = 1000;
    
    CircularQueue<int, kQueueSize> queue;
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    std::vector<std::atomic<size_t>> consumer_counters(kNumConsumers);
    std::atomic<size_t> total_messages(0);
    std::atomic<bool> done(false);
    
    // 初始化计数器
    for (auto& counter : consumer_counters) {
        counter = 0;
    }
    
    // 创建消费者线程
    for (size_t i = 0; i < kNumConsumers; ++i) {
        size_t consumer_id = queue.AddConsumer();
        consumers.emplace_back([&, consumer_id, i]() {
            int value = 0;
            while (!done || !queue.Empty(consumer_id)) {
                if (queue.Pop(value, consumer_id)) {
                    consumer_counters[i]++;
                    total_messages++;
                } else {
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
            }
        });
    }
    
    // 创建生产者线程
    for (size_t i = 0; i < kNumProducers; ++i) {
        producers.emplace_back([&, i]() {
            for (int j = 1; j <= static_cast<int>(kNumMessagesPerProducer); ++j) {
                int value = i * kNumMessagesPerProducer + j;
                while (!queue.Push(value)) {
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
            }
        });
    }
    
    // 等待生产者完成
    for (auto& producer : producers) {
        producer.join();
    }
    
    // 等待所有消费者处理完消息
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // 设置done标志并等待消费者线程结束
    done = true;
    for (auto& consumer : consumers) {
        consumer.join();
    }
    
    // 验证每个消费者都收到了所有消息
    for (size_t i = 0; i < kNumConsumers; ++i) {
        EXPECT_EQ(consumer_counters[i], kNumProducers * kNumMessagesPerProducer);
    }
    
    // 验证总消息数是生产者产生的消息数乘以消费者数量
    EXPECT_EQ(total_messages, kNumProducers * kNumMessagesPerProducer * kNumConsumers);
}
