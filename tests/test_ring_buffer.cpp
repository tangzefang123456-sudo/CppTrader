#include "test.h"
#include "trader/common/ring_buffer.h"

#include <thread>
#include <vector>
#include <atomic>

using namespace trader::common;

TEST(RingBuffer, SingleProducerSingleConsumer) {
    const size_t capacity = 1024;
    RingBuffer<int> ring_buffer(capacity);
    auto consumer = ring_buffer.CreateConsumer();

    const int num_items = 512;
    int received_items = 0;

    // 生产者线程
    std::thread producer([&]() {
        for (int i = 0; i < num_items; ++i) {
            while (!ring_buffer.Write(i)) {
                // 队列满，等待
            }
        }
    });

    // 消费者线程
    std::thread consumer_thread([&]() {
        int data;
        while (received_items < num_items) {
            if (consumer->Read(data)) {
                EXPECT_EQ(data, received_items);
                ++received_items;
            }
        }
    });

    producer.join();
    consumer_thread.join();

    EXPECT_EQ(received_items, num_items);
    EXPECT_EQ(ring_buffer.Size(), num_items);
}

TEST(RingBuffer, SingleProducerMultipleConsumers) {
    const size_t capacity = 1024;
    RingBuffer<int> ring_buffer(capacity);

    const int num_consumers = 4;
    const int num_items = 512;
    std::vector<std::atomic<int>> received_items(num_consumers, 0);
    std::vector<std::unique_ptr<RingBuffer<int>::Consumer>> consumers;
    std::vector<std::thread> consumer_threads;

    // 创建消费者
    for (int i = 0; i < num_consumers; ++i) {
        consumers.push_back(ring_buffer.CreateConsumer());
    }

    // 生产者线程
    std::thread producer([&]() {
        for (int i = 0; i < num_items; ++i) {
            while (!ring_buffer.Write(i)) {
                // 队列满，等待
            }
        }
    });

    // 消费者线程
    for (int i = 0; i < num_consumers; ++i) {
        consumer_threads.emplace_back([&, i]() {
            int data;
            while (received_items[i] < num_items) {
                if (consumers[i]->Read(data)) {
                    EXPECT_EQ(data, received_items[i]);
                    ++received_items[i];
                }
            }
        });
    }

    producer.join();
    for (auto& thread : consumer_threads) {
        thread.join();
    }

    // 检查每个消费者都收到了所有数据
    for (int i = 0; i < num_consumers; ++i) {
        EXPECT_EQ(received_items[i], num_items);
    }
}

TEST(RingBuffer, WriteWhenFull) {
    const size_t capacity = 4;
    RingBuffer<int> ring_buffer(capacity);

    // 填满队列
    for (int i = 0; i < capacity; ++i) {
        EXPECT_TRUE(ring_buffer.Write(i));
    }

    // 队列已满，写入失败
    EXPECT_FALSE(ring_buffer.Write(capacity));
    EXPECT_FALSE(ring_buffer.Write(capacity + 1));

    // 读取一个数据
    auto consumer = ring_buffer.CreateConsumer();
    int data;
    EXPECT_TRUE(consumer->Read(data));
    EXPECT_EQ(data, 0);

    // 现在可以写入一个数据
    EXPECT_TRUE(ring_buffer.Write(capacity));
}

TEST(RingBuffer, MoveWrite) {
    const size_t capacity = 1024;
    RingBuffer<std::string> ring_buffer(capacity);
    auto consumer = ring_buffer.CreateConsumer();

    const int num_items = 512;
    int received_items = 0;

    // 生产者线程
    std::thread producer([&]() {
        for (int i = 0; i < num_items; ++i) {
            std::string str = "item " + std::to_string(i);
            while (!ring_buffer.Write(std::move(str))) {
                // 队列满，等待
                str = "item " + std::to_string(i);
            }
        }
    });

    // 消费者线程
    std::thread consumer_thread([&]() {
        std::string data;
        while (received_items < num_items) {
            if (consumer->Read(data)) {
                EXPECT_EQ(data, "item " + std::to_string(received_items));
                ++received_items;
            }
        }
    });

    producer.join();
    consumer_thread.join();

    EXPECT_EQ(received_items, num_items);
}

TEST(RingBuffer, CapacityAndSize) {
    const size_t capacity = 1024;
    RingBuffer<int> ring_buffer(capacity);

    EXPECT_EQ(ring_buffer.Capacity(), capacity);
    EXPECT_EQ(ring_buffer.Size(), 0);
    EXPECT_TRUE(ring_buffer.Empty());
    EXPECT_FALSE(ring_buffer.Full());

    // 写入一个数据
    EXPECT_TRUE(ring_buffer.Write(42));
    EXPECT_EQ(ring_buffer.Size(), 1);
    EXPECT_FALSE(ring_buffer.Empty());
    EXPECT_FALSE(ring_buffer.Full());

    // 填满队列
    for (int i = 1; i < capacity; ++i) {
        EXPECT_TRUE(ring_buffer.Write(i));
    }

    EXPECT_EQ(ring_buffer.Size(), capacity);
    EXPECT_FALSE(ring_buffer.Empty());
    EXPECT_TRUE(ring_buffer.Full());
}
