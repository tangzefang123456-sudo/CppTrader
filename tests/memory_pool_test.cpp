#include <gtest/gtest.h>
#include <common/memory_pool.h>
#include <thread>
#include <vector>

// 测试用结构体
struct TestStruct {
    int id;
    std::string name;
    double value;

    TestStruct() : id(0), name(""), value(0.0) {}
    TestStruct(int i, const std::string& n, double v) : id(i), name(n), value(v) {}
};

// 测试用类
class TestClass {
public:
    TestClass() : m_data(0) {}
    TestClass(int data) : m_data(data) {}
    ~TestClass() {}

    int get_data() const { return m_data; }
    void set_data(int data) { m_data = data; }

private:
    int m_data;
};

// 基本功能测试
TEST(MemoryPoolTest, BasicFunctionality) {
    common::MemoryPool<TestStruct> pool(2);

    // 检查初始状态
    EXPECT_EQ(pool.total_count(), 2);
    EXPECT_EQ(pool.free_count(), 2);
    EXPECT_EQ(pool.used_count(), 0);

    // 分配第一个对象
    TestStruct* obj1 = pool.allocate();
    EXPECT_NE(obj1, nullptr);
    EXPECT_EQ(pool.total_count(), 2);
    EXPECT_EQ(pool.free_count(), 1);
    EXPECT_EQ(pool.used_count(), 1);

    // 分配第二个对象
    TestStruct* obj2 = pool.allocate();
    EXPECT_NE(obj2, nullptr);
    EXPECT_EQ(pool.total_count(), 2);
    EXPECT_EQ(pool.free_count(), 0);
    EXPECT_EQ(pool.used_count(), 2);

    // 分配第三个对象，应该触发扩容
    TestStruct* obj3 = pool.allocate();
    EXPECT_NE(obj3, nullptr);
    EXPECT_EQ(pool.total_count(), 4); // 2 * 2
    EXPECT_EQ(pool.free_count(), 1);
    EXPECT_EQ(pool.used_count(), 3);

    // 释放对象
    pool.deallocate(obj1);
    EXPECT_EQ(pool.free_count(), 2);
    EXPECT_EQ(pool.used_count(), 2);

    pool.deallocate(obj2);
    EXPECT_EQ(pool.free_count(), 3);
    EXPECT_EQ(pool.used_count(), 1);

    pool.deallocate(obj3);
    EXPECT_EQ(pool.free_count(), 4);
    EXPECT_EQ(pool.used_count(), 0);
}

// 构造和销毁功能测试
TEST(MemoryPoolTest, ConstructAndDestroy) {
    common::MemoryPool<TestClass> pool(3);

    // 使用construct创建对象
    TestClass* obj1 = pool.construct(10);
    EXPECT_NE(obj1, nullptr);
    EXPECT_EQ(obj1->get_data(), 10);
    EXPECT_EQ(pool.used_count(), 1);

    TestClass* obj2 = pool.construct(20);
    EXPECT_NE(obj2, nullptr);
    EXPECT_EQ(obj2->get_data(), 20);
    EXPECT_EQ(pool.used_count(), 2);

    // 使用destroy销毁对象
    pool.destroy(obj1);
    EXPECT_EQ(pool.used_count(), 1);

    pool.destroy(obj2);
    EXPECT_EQ(pool.used_count(), 0);
}

// 线程安全测试
TEST(MemoryPoolTest, ThreadSafety) {
    const int thread_count = 8;
    const int objects_per_thread = 1000;
    common::MemoryPool<TestStruct> pool(16);
    std::vector<std::thread> threads;

    // 多个线程同时分配和释放对象
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([&pool, i, objects_per_thread]() {
            std::vector<TestStruct*> objects;
            objects.reserve(objects_per_thread);

            // 分配对象
            for (int j = 0; j < objects_per_thread; ++j) {
                TestStruct* obj = pool.construct(i * objects_per_thread + j, "test", 3.14);
                EXPECT_NE(obj, nullptr);
                objects.push_back(obj);
            }

            // 验证对象数据
            for (int j = 0; j < objects_per_thread; ++j) {
                TestStruct* obj = objects[j];
                EXPECT_EQ(obj->id, i * objects_per_thread + j);
                EXPECT_EQ(obj->name, "test");
                EXPECT_DOUBLE_EQ(obj->value, 3.14);
            }

            // 释放对象
            for (TestStruct* obj : objects) {
                pool.destroy(obj);
            }
        });
    }

    // 等待所有线程完成
    for (std::thread& t : threads) {
        t.join();
    }

    // 所有对象都应该被释放
    EXPECT_EQ(pool.used_count(), 0);
}

// 扩容测试
TEST(MemoryPoolTest, Expansion) {
    common::MemoryPool<TestClass> pool(2);

    EXPECT_EQ(pool.total_count(), 2);

    // 分配2个对象，耗尽内存
    TestClass* obj1 = pool.allocate();
    TestClass* obj2 = pool.allocate();
    EXPECT_EQ(pool.free_count(), 0);

    // 分配第3个对象，触发扩容到4个节点
    TestClass* obj3 = pool.allocate();
    EXPECT_EQ(pool.total_count(), 4);
    EXPECT_EQ(pool.free_count(), 1);

    // 分配第4个对象
    TestClass* obj4 = pool.allocate();
    EXPECT_EQ(pool.free_count(), 0);

    // 分配第5个对象，触发扩容到8个节点
    TestClass* obj5 = pool.allocate();
    EXPECT_EQ(pool.total_count(), 8);
    EXPECT_EQ(pool.free_count(), 3);

    // 释放所有对象
    pool.deallocate(obj1);
    pool.deallocate(obj2);
    pool.deallocate(obj3);
    pool.deallocate(obj4);
    pool.deallocate(obj5);

    EXPECT_EQ(pool.free_count(), 8);
    EXPECT_EQ(pool.used_count(), 0);
}

// 无效指针测试
TEST(MemoryPoolTest, InvalidPointer) {
    common::MemoryPool<TestClass> pool(2);

    // 尝试释放空指针
    pool.deallocate(nullptr); // 应该不会崩溃

    // 尝试释放非内存池中的指针
    TestClass* external_obj = new TestClass();
    EXPECT_THROW(pool.deallocate(external_obj), std::invalid_argument);
    delete external_obj;

    // 正常分配和释放
    TestClass* obj = pool.allocate();
    pool.deallocate(obj); // 应该成功
}

// 性能测试
TEST(MemoryPoolTest, Performance) {
    const int object_count = 100000;
    common::MemoryPool<TestStruct> pool(1024);
    std::vector<TestStruct*> objects;
    objects.reserve(object_count);

    // 测试分配性能
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < object_count; ++i) {
        TestStruct* obj = pool.construct(i, "performance", 1.234);
        objects.push_back(obj);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Allocated " << object_count << " objects in " << duration.count() << " ms" << std::endl;

    // 测试释放性能
    start = std::chrono::high_resolution_clock::now();
    for (TestStruct* obj : objects) {
        pool.destroy(obj);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Freed " << object_count << " objects in " << duration.count() << " ms" << std::endl;

    EXPECT_EQ(pool.used_count(), 0);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
