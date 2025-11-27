# C++ 内存池通用组件

一个跨平台的C++内存池通用组件，支持Windows和Linux平台，提供高效的内存管理功能。

## 功能特性

- ✅ 跨平台支持：Windows (Visual Studio 2022) 和 Linux
- ✅ 模板实现：支持任意结构体或类类型
- ✅ 自动扩容：当节点个数不够时，自动双倍扩充
- ✅ 线程安全：支持多线程并发访问
- ✅ 高效性能：减少内存分配和释放的开销
- ✅ 完整的单元测试：使用Google Test进行全面测试

## 设计原理

### 内存池结构

内存池使用一块连续的内存区域来存储对象节点，每个节点包含：
- 对象数据本身
- 前向指针（next）
- 后向指针（prev）

### 空闲列表

内存池维护一个空闲节点列表，使用双链表结构管理空闲节点。当需要分配对象时，从空闲列表头部取出一个节点；当释放对象时，将节点放回空闲列表头部。

### 自动扩容

当空闲列表为空时，内存池会自动双倍扩容：
1. 分配一块新的、更大的内存区域
2. 将原有内存中的数据复制到新区域
3. 初始化新分配的节点并加入空闲列表
4. 释放原有内存区域

## 使用方法

### 基本用法

```cpp
#include <common/memory_pool.h>

// 定义一个测试结构体
struct TestStruct {
    int id;
    std::string name;
    double value;
};

int main() {
    // 创建内存池，初始大小为16个节点
    common::MemoryPool<TestStruct> pool(16);

    // 分配对象
    TestStruct* obj1 = pool.allocate();
    obj1->id = 1;
    obj1->name = "test1";
    obj1->value = 3.14;

    // 使用construct直接构造对象
    TestStruct* obj2 = pool.construct(2, "test2", 6.28);

    // 释放对象
    pool.deallocate(obj1);

    // 使用destroy直接销毁对象
    pool.destroy(obj2);

    return 0;
}
```

### 线程安全

内存池内部使用互斥锁保证线程安全，可以在多线程环境中放心使用：

```cpp
#include <common/memory_pool.h>
#include <thread>
#include <vector>

common::MemoryPool<int> pool(100);

void thread_function(int thread_id) {
    for (int i = 0; i < 1000; ++i) {
        int* value = pool.construct(thread_id * 1000 + i);
        // 使用value...
        pool.destroy(value);
    }
}

int main() {
    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back(thread_function, i);
    }
    for (auto& t : threads) {
        t.join();
    }
    return 0;
}
```

## 项目结构

```
├── include/
│   └── common/
│       └── memory_pool.h    # 内存池头文件
├── src/
│   └── common/
│       └── memory_pool.cpp  # 内存池实现文件
├── tests/
│   └── memory_pool_test.cpp # 单元测试文件
├── MemoryPool.sln           # Visual Studio解决方案
├── MemoryPool.vcxproj       # Visual Studio项目文件
├── MemoryPoolTest.vcxproj   # 测试项目文件
├── CMakeLists.txt           # CMake配置文件
├── build.sh                 # Linux编译脚本
├── compile.bat              # Windows编译脚本
├── Makefile                 # Makefile配置
└── README_MEMORY_POOL.md    # 本文件
```

## 编译和运行

### Windows平台

1. 使用Visual Studio 2022打开`MemoryPool.sln`解决方案
2. 编译`MemoryPool`项目（静态库）
3. 编译并运行`MemoryPoolTest`项目（单元测试）

或者使用批处理脚本：

```batch
build.bat
```

### Linux平台

```bash
chmod +x build.sh
./build.sh
```

## 单元测试

单元测试使用Google Test框架，覆盖以下功能：

- 基本功能测试
- 构造和销毁功能测试
- 线程安全测试
- 扩容测试
- 无效指针测试
- 性能测试

## 性能优势

内存池的主要性能优势在于：

1. **减少内存碎片**：使用连续内存区域存储对象
2. **减少系统调用**：一次性分配大块内存，减少malloc/free调用
3. **快速分配/释放**：通过空闲列表实现O(1)时间复杂度的分配和释放
4. **缓存友好**：对象存储在连续内存中，提高CPU缓存命中率

## 注意事项

1. 内存池中的对象必须是POD类型或具有默认构造函数的类型
2. 不要手动释放内存池分配的对象，必须使用`deallocate`或`destroy`方法
3. 不要将非内存池分配的对象传递给`deallocate`方法
4. 内存池不支持对象的移动或拷贝操作

## 许可证

MIT License
