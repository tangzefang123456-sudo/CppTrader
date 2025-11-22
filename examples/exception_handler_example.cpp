#include "trader/common/ExceptionHandler.h"

#include <iostream>
#include <thread>
#include <memory>
#include <chrono>
#include <atomic>

// 全局资源示例
std::unique_ptr<std::thread> g_workerThread;
std::atomic<bool> g_isRunning(false);

// 资源释放回调函数
void CleanupResources() {
    std::cout << "[Cleanup] Starting resource cleanup..." << std::endl;
    
    // 停止工作线程
    g_isRunning = false;
    if (g_workerThread && g_workerThread->joinable()) {
        std::cout << "[Cleanup] Joining worker thread..." << std::endl;
        g_workerThread->join();
        g_workerThread.reset();
    }
    
    // 释放其他资源示例
    std::cout << "[Cleanup] All resources cleaned up successfully!" << std::endl;
}

// 工作线程函数 - 模拟可能发生异常的工作
void WorkerThread() {
    std::cout << "[Worker] Worker thread started (Thread ID: " << std::this_thread::get_id() << ")" << std::endl;
    g_isRunning = true;
    
    try {
        int counter = 0;
        while (g_isRunning) {
            std::cout << "[Worker] Processing task " << counter++ << std::endl;
            
            // 模拟一些工作
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            // 在第5个任务时模拟段错误
            if (counter == 5) {
                std::cout << "[Worker] Simulating segmentation fault..." << std::endl;
                int* p = nullptr;
                *p = 42; // 这会导致段错误
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[Worker] Exception caught: " << e.what() << std::endl;
    }
    
    std::cout << "[Worker] Worker thread ended" << std::endl;
}

// 另一个可能导致异常的函数
void CauseFloatingPointException() {
    std::cout << "[Main] Causing floating point exception..." << std::endl;
    int x = 0;
    int y = 10 / x; // 这会导致浮点异常
}

int main() {
    std::cout << "[Main] Main thread started (Thread ID: " << std::this_thread::get_id() << ")" << std::endl;
    
    // 初始化异常处理器
    trader::common::ExceptionHandler& exceptionHandler = trader::common::ExceptionHandler::GetInstance();
    
    // 设置日志文件路径和最大大小（5MB）
    if (!exceptionHandler.Init("exception_logs/exception.log", 5 * 1024 * 1024)) {
        std::cerr << "[Main] Failed to initialize exception handler!" << std::endl;
        return 1;
    }
    
    // 注册资源释放回调
    exceptionHandler.RegisterCleanupCallback(CleanupResources);
    
    // 启动异常捕获
    if (!exceptionHandler.Start()) {
        std::cerr << "[Main] Failed to start exception handler!" << std::endl;
        return 1;
    }
    
    std::cout << "[Main] Exception handler started successfully" << std::endl;
    
    // 选项菜单
    int choice = 0;
    do {
        std::cout << std::endl;
        std::cout << "=== Exception Handler Demo ===" << std::endl;
        std::cout << "1. Start worker thread (will crash after 5 tasks)" << std::endl;
        std::cout << "2. Cause floating point exception" << std::endl;
        std::cout << "3. Exit normally" << std::endl;
        std::cout << "Enter your choice: ";
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                if (!g_workerThread) {
                    g_workerThread = std::make_unique<std::thread>(WorkerThread);
                    std::cout << "[Main] Worker thread created" << std::endl;
                } else {
                    std::cout << "[Main] Worker thread is already running" << std::endl;
                }
                break;
                
            case 2:
                CauseFloatingPointException();
                break;
                
            case 3:
                std::cout << "[Main] Exiting normally..." << std::endl;
                break;
                
            default:
                std::cout << "[Main] Invalid choice, please try again" << std::endl;
                break;
        }
    } while (choice != 3);
    
    // 停止异常捕获
    exceptionHandler.Stop();
    
    // 手动清理资源
    CleanupResources();
    
    std::cout << "[Main] Main thread ended successfully" << std::endl;
    
    return 0;
}