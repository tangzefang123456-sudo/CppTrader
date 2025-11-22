# C++异常捕获基础组件

## 概述

这是一个C++异常捕获基础组件，用于捕获程序中的段错误等异常，并生成详细的堆栈信息写入日志文件。组件提供了Init、Start、Stop接口，支持注册资源释放回调函数，确保异常发生后能正确释放所有资源。

## 功能特性

1. **异常捕获**：捕获段错误(SIGSEGV)、非法指令(SIGILL)、浮点异常(SIGFPE)、 abort信号(SIGABRT)、总线错误(SIGBUS)和系统调用错误(SIGSYS)等异常
2. **堆栈跟踪**：生成详细的堆栈跟踪信息，包括函数调用链
3. **日志记录**：将异常信息写入日志文件，支持日志文件滚动
4. **资源释放**：支持注册资源释放回调函数，确保异常发生后能正确释放所有资源
5. **跨平台支持**：支持Windows和Linux平台

## 接口说明

### 单例模式

```cpp
trader::common::ExceptionHandler& handler = trader::common::ExceptionHandler::GetInstance();
```

### 初始化

```cpp
bool Init(const std::string& logFilePath, size_t maxLogSize = 10 * 1024 * 1024);
```

- `logFilePath`：日志文件路径
- `maxLogSize`：最大日志文件大小（字节），默认10MB
- 返回值：初始化成功返回true，失败返回false

### 启动异常捕获

```cpp
bool Start();
```

- 返回值：启动成功返回true，失败返回false

### 停止异常捕获

```cpp
bool Stop();
```

- 返回值：停止成功返回true，失败返回false

### 注册资源释放回调函数

```cpp
void RegisterCleanupCallback(CleanupCallback callback);
```

- `callback`：资源释放回调函数

### 移除所有注册的资源释放回调函数

```cpp
void UnregisterAllCleanupCallbacks();
```

### 获取异常信息

```cpp
const std::vector<std::string>& GetLastStackTrace() const;
const std::string& GetLastExceptionType() const;
const std::string& GetLastExceptionTime() const;
```

## 使用示例

```cpp
#include "trader/common/ExceptionHandler.h"

#include <iostream>
#include <thread>

// 全局资源
std::thread* g_workerThread = nullptr;

// 资源释放回调函数
void CleanupResources() {
    std::cout << "Cleaning up resources..." << std::endl;
    
    // 停止工作线程
    if (g_workerThread != nullptr && g_workerThread->joinable()) {
        g_workerThread->join();
        delete g_workerThread;
        g_workerThread = nullptr;
    }
    
    // 释放其他资源...
}

// 工作线程函数
void WorkerThread() {
    std::cout << "Worker thread started" << std::endl;
    
    // 模拟段错误
    int* p = nullptr;
    *p = 42;
    
    std::cout << "Worker thread ended" << std::endl;
}

int main() {
    std::cout << "Main thread started" << std::endl;
    
    // 初始化异常处理器
    trader::common::ExceptionHandler& handler = trader::common::ExceptionHandler::GetInstance();
    if (!handler.Init("exception.log", 10 * 1024 * 1024)) {
        std::cerr << "Failed to initialize exception handler" << std::endl;
        return 1;
    }
    
    // 注册资源释放回调
    handler.RegisterCleanupCallback(CleanupResources);
    
    // 启动异常捕获
    if (!handler.Start()) {
        std::cerr << "Failed to start exception handler" << std::endl;
        return 1;
    }
    
    // 创建工作线程
    g_workerThread = new std::thread(WorkerThread);
    
    // 等待工作线程结束
    if (g_workerThread->joinable()) {
        g_workerThread->join();
        delete g_workerThread;
        g_workerThread = nullptr;
    }
    
    // 停止异常捕获
    handler.Stop();
    
    std::cout << "Main thread ended" << std::endl;
    
    return 0;
}
```

## 构建说明

### 生成Visual Studio解决方案

```batch
generate_solution.bat
```

### 编译项目

```batch
build_project.bat
```

### 运行单元测试

```batch
cd build
test_exception_handler.exe
```

## 日志格式

异常日志文件包含以下信息：

- 异常发生时间
- 异常类型（信号类型）
- 进程ID
- 线程ID
- 详细的堆栈跟踪信息

示例日志：

```
===========================================
Exception Time: 2023-10-01 12:34:56
Signal: 11 - Segmentation Fault (SIGSEGV)
Process ID: 1234
Thread ID: 5678
Stack Trace:
  0: ./test_exception_handler() [0x401234]
  1: ./test_exception_handler() [0x401356]
  2: ./test_exception_handler() [0x401478]
  3: /lib64/libc.so.6(+0x23456) [0x7f89abcdefgh]
  4: /lib64/libc.so.6(__libc_start_main+0x123) [0x7f89abcde123]
  5: ./test_exception_handler() [0x401000]
===========================================
```

## 注意事项

1. 异常捕获组件必须在程序启动时尽早初始化和启动
2. 资源释放回调函数中应避免使用可能导致异常的代码
3. 异常发生后，程序会立即退出，因此资源释放回调函数应尽可能简洁高效
4. 在Windows平台上，需要确保程序以调试模式编译，以便生成正确的堆栈跟踪信息
5. 在Linux平台上，需要链接pthread库（-lpthread）