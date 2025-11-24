//
// Created by Your Name on 2024-05-20
//

#include "trader/common/ExceptionHandler.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <fstream>
#include <filesystem>

using namespace Trader::Common;

// 全局资源
std::thread* g_workerThread = nullptr;
std::atomic<bool> g_isRunning = false;

// 资源释放回调函数
void CleanupResources() {
    std::cout << "Cleaning up resources..." << std::endl;

    // 停止工作线程
    if (g_isRunning) {
        g_isRunning = false;
        if (g_workerThread && g_workerThread->joinable()) {
            g_workerThread->join();
            delete g_workerThread;
            g_workerThread = nullptr;
        }
    }

    std::cout << "Resources cleaned up successfully." << std::endl;
}

// 工作线程函数
void WorkerThread() {
    std::cout << "Worker thread started." << std::endl;

    while (g_isRunning) {
        // 模拟工作
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "Worker thread stopped." << std::endl;
}

// 模拟段错误
void CauseSegFault() {
    std::cout << "Causing segmentation fault..." << std::endl;
    int* p = nullptr;
    *p = 42;  // 这会导致段错误
}

// 模拟浮点异常
void CauseFloatingPointException() {
    std::cout << "Causing floating point exception..." << std::endl;
    int a = 1;
    int b = 0;
    int c = a / b;  // 这会导致浮点异常
}

int main() {
    std::cout << "Exception Handler Example" << std::endl;
    std::cout << "========================" << std::endl;

    // 初始化异常捕获组件
    ExceptionHandler& handler = ExceptionHandler::GetInstance();
    if (!handler.Init("logs/exception.log")) {
        std::cerr << "Failed to initialize exception handler." << std::endl;
        return 1;
    }

    // 注册资源释放回调函数
    handler.RegisterCleanupCallback(CleanupResources);

    // 启动异常捕获组件
    if (!handler.Start()) {
        std::cerr << "Failed to start exception handler." << std::endl;
        return 1;
    }

    // 启动工作线程
    g_isRunning = true;
    g_workerThread = new std::thread(WorkerThread);

    // 提示用户选择要测试的异常类型
    std::cout << std::endl;
    std::cout << "Select an exception type to test:" << std::endl;
    std::cout << "1. Segmentation Fault" << std::endl;
    std::cout << "2. Floating Point Exception" << std::endl;
    std::cout << "3. Exit" << std::endl;

    int choice;
    std::cin >> choice;

    switch (choice) {
        case 1:
            CauseSegFault();
            break;
        case 2:
            CauseFloatingPointException();
            break;
        case 3:
            // 正常退出
            CleanupResources();
            return 0;
        default:
            std::cout << "Invalid choice. Exiting..." << std::endl;
            CleanupResources();
            return 0;
    }

    // 程序应该不会执行到这里，因为异常会导致程序退出
    return 0;
}
