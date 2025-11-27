#include "test.h"
#include "trader/common/memory_pool.h"

using namespace trader::common;

// 测试用结构体
struct TestStruct {
    int id;
    std::string name;
    double value;
    
    TestStruct() : id(0), name(""), value(0.0) {}
    TestStruct(int id, const std::string& name, double value) 
        : id(id), name(name), value(value) {}
};

// 测试内存池的基本功能
TEST(MemoryPool, BasicFunctionality) {
    MemoryPool<TestStruct> pool(10);
    
    // 测试初始大小
    EXPECT_EQ(pool.Size(), 10U);
    EXPECT_EQ(pool.AllocatedSize(), 0U);
    
    // 分配第一个节点
    MemoryPoolNode<TestStruct>* node1 = pool.Allocate();
    EXPECT_NE(node1, nullptr);
    EXPECT_EQ(pool.AllocatedSize(), 1U);
    
    // 分配第二个节点
    MemoryPoolNode<TestStruct>* node2 = pool.Allocate();
    EXPECT_NE(node2, nullptr);
    EXPECT_EQ(pool.AllocatedSize(), 2U);
    
    // 测试节点数据访问
    node1->data = TestStruct(1, "Node1", 1.1);
    node2->data = TestStruct(2, "Node2", 2.2);
    
    EXPECT_EQ(node1->data.id, 1);
    EXPECT_EQ(node1->data.name, "Node1");
    EXPECT_DOUBLE_EQ(node1->data.value, 1.1);
    
    EXPECT_EQ(node2->data.id, 2);
    EXPECT_EQ(node2->data.name, "Node2");
    EXPECT_DOUBLE_EQ(node2->data.value, 2.2);
    
    // 释放节点
    pool.Deallocate(node1);
    EXPECT_EQ(pool.AllocatedSize(), 1U);
    
    pool.Deallocate(node2);
    EXPECT_EQ(pool.AllocatedSize(), 0U);
    
    // 重新分配节点
    MemoryPoolNode<TestStruct>* node3 = pool.Allocate();
    EXPECT_NE(node3, nullptr);
    EXPECT_EQ(pool.AllocatedSize(), 1U);
}

// 测试内存池的自动扩容功能
TEST(MemoryPool, AutoExpand) {
    MemoryPool<TestStruct> pool(5);
    
    EXPECT_EQ(pool.Size(), 5U);
    
    // 分配6个节点，触发扩容
    MemoryPoolNode<TestStruct>* nodes[6];
    for (int i = 0; i < 6; ++i) {
        nodes[i] = pool.Allocate();
        EXPECT_NE(nodes[i], nullptr);
    }
    
    // 扩容后大小应该是5 + 10 = 15
    EXPECT_EQ(pool.Size(), 15U);
    EXPECT_EQ(pool.AllocatedSize(), 6U);
    
    // 释放所有节点
    for (int i = 0; i < 6; ++i) {
        pool.Deallocate(nodes[i]);
    }
    
    EXPECT_EQ(pool.AllocatedSize(), 0U);
}

// 测试内存池的线程安全
TEST(MemoryPool, ThreadSafety) {
    const int kThreadCount = 10;
    const int kAllocationsPerThread = 100;
    
    MemoryPool<TestStruct> pool(100);
    std::vector<std::thread> threads;
    
    // 多个线程同时分配和释放节点
    for (int i = 0; i < kThreadCount; ++i) {
        threads.emplace_back([&pool, kAllocationsPerThread, i]() {
            std::vector<MemoryPoolNode<TestStruct>*> nodes;
            
            // 分配节点
            for (int j = 0; j < kAllocationsPerThread; ++j) {
                MemoryPoolNode<TestStruct>* node = pool.Allocate();
                EXPECT_NE(node, nullptr);
                
                // 设置节点数据
                node->data = TestStruct(i * kAllocationsPerThread + j, 
                                      "Thread" + std::to_string(i) + "Node" + std::to_string(j),
                                      static_cast<double>(i * kAllocationsPerThread + j));
                
                nodes.push_back(node);
            }
            
            // 验证节点数据
            for (int j = 0; j < kAllocationsPerThread; ++j) {
                MemoryPoolNode<TestStruct>* node = nodes[j];
                EXPECT_EQ(node->data.id, i * kAllocationsPerThread + j);
                EXPECT_EQ(node->data.name, "Thread" + std::to_string(i) + "Node" + std::to_string(j));
                EXPECT_DOUBLE_EQ(node->data.value, static_cast<double>(i * kAllocationsPerThread + j));
            }
            
            // 释放节点
            for (MemoryPoolNode<TestStruct>* node : nodes) {
                pool.Deallocate(node);
            }
        });
    }
    
    // 等待所有线程完成
    for (std::thread& thread : threads) {
        thread.join();
    }
    
    // 所有节点应该都已释放
    EXPECT_EQ(pool.AllocatedSize(), 0U);
}

// 测试内存池的边界情况
TEST(MemoryPool, EdgeCases) {
    // 测试初始大小为1
    MemoryPool<TestStruct> pool1(1);
    EXPECT_EQ(pool1.Size(), 1U);
    
    MemoryPoolNode<TestStruct>* node1 = pool1.Allocate();
    EXPECT_NE(node1, nullptr);
    EXPECT_EQ(pool1.AllocatedSize(), 1U);
    
    // 再次分配应该触发扩容
    MemoryPoolNode<TestStruct>* node2 = pool1.Allocate();
    EXPECT_NE(node2, nullptr);
    EXPECT_EQ(pool1.Size(), 3U); // 1 + 2 = 3
    EXPECT_EQ(pool1.AllocatedSize(), 2U);
    
    pool1.Deallocate(node1);
    pool1.Deallocate(node2);
    
    // 测试分配和释放大量节点
    MemoryPool<TestStruct> pool2(100);
    std::vector<MemoryPoolNode<TestStruct>*> nodes;
    
    for (int i = 0; i < 1000; ++i) {
        nodes.push_back(pool2.Allocate());
    }
    
    EXPECT_GE(pool2.Size(), 1000U);
    EXPECT_EQ(pool2.AllocatedSize(), 1000U);
    
    for (MemoryPoolNode<TestStruct>* node : nodes) {
        pool2.Deallocate(node);
    }
    
    EXPECT_EQ(pool2.AllocatedSize(), 0U);
}
