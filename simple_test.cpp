#include <common/memory_pool.h>
#include <iostream>
#include <vector>

// 测试用结构体
struct TestStruct {
    int id;
    std::string name;
    double value;

    TestStruct() : id(0), name(""), value(0.0) {
        std::cout << "TestStruct default constructor" << std::endl;
    }
    TestStruct(int i, const std::string& n, double v) : id(i), name(n), value(v) {
        std::cout << "TestStruct constructor: id=" << id << std::endl;
    }
    ~TestStruct() {
        std::cout << "TestStruct destructor: id=" << id << std::endl;
    }
};

int main() {
    std::cout << "=== 内存池测试开始 ===" << std::endl;

    try {
        // 创建内存池，初始大小为2个节点
        common::MemoryPool<TestStruct> pool(2);

        std::cout << "初始状态: 总节点数=" << pool.total_count() << ", 空闲节点数=" << pool.free_count() << std::endl;

        // 分配第一个对象
        TestStruct* obj1 = pool.allocate();
        obj1->id = 1;
        obj1->name = "test1";
        obj1->value = 3.14;
        std::cout << "分配对象1后: 总节点数=" << pool.total_count() << ", 空闲节点数=" << pool.free_count() << std::endl;

        // 分配第二个对象
        TestStruct* obj2 = pool.allocate();
        obj2->id = 2;
        obj2->name = "test2";
        obj2->value = 6.28;
        std::cout << "分配对象2后: 总节点数=" << pool.total_count() << ", 空闲节点数=" << pool.free_count() << std::endl;

        // 分配第三个对象，触发扩容
        TestStruct* obj3 = pool.allocate();
        obj3->id = 3;
        obj3->name = "test3";
        obj3->value = 9.42;
        std::cout << "分配对象3后(触发扩容): 总节点数=" << pool.total_count() << ", 空闲节点数=" << pool.free_count() << std::endl;

        // 使用construct构造对象
        TestStruct* obj4 = pool.construct(4, "test4", 12.56);
        std::cout << "构造对象4后: 总节点数=" << pool.total_count() << ", 空闲节点数=" << pool.free_count() << std::endl;

        // 验证对象数据
        std::cout << "对象1: id=" << obj1->id << ", name=" << obj1->name << ", value=" << obj1->value << std::endl;
        std::cout << "对象2: id=" << obj2->id << ", name=" << obj2->name << ", value=" << obj2->value << std::endl;
        std::cout << "对象3: id=" << obj3->id << ", name=" << obj3->name << ", value=" << obj3->value << std::endl;
        std::cout << "对象4: id=" << obj4->id << ", name=" << obj4->name << ", value=" << obj4->value << std::endl;

        // 释放对象
        pool.deallocate(obj1);
        std::cout << "释放对象1后: 总节点数=" << pool.total_count() << ", 空闲节点数=" << pool.free_count() << std::endl;

        pool.destroy(obj4);
        std::cout << "销毁对象4后: 总节点数=" << pool.total_count() << ", 空闲节点数=" << pool.free_count() << std::endl;

        // 分配更多对象，测试多次扩容
        std::vector<TestStruct*> objects;
        for (int i = 5; i < 20; ++i) {
            TestStruct* obj = pool.construct(i, "test" + std::to_string(i), i * 3.14);
            objects.push_back(obj);
        }
        std::cout << "分配多个对象后: 总节点数=" << pool.total_count() << ", 空闲节点数=" << pool.free_count() << std::endl;

        // 释放所有对象
        for (TestStruct* obj : objects) {
            pool.destroy(obj);
        }
        pool.deallocate(obj2);
        pool.deallocate(obj3);
        std::cout << "释放所有对象后: 总节点数=" << pool.total_count() << ", 空闲节点数=" << pool.free_count() << std::endl;

        std::cout << "=== 内存池测试完成 ===" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "测试失败: " << e.what() << std::endl;
        return 1;
    }
}
