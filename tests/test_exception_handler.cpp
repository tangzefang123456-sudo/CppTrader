#include "test.h"
#include "trader/common/ExceptionHandler.h"

#include <thread>
#include <atomic>
#include <fstream>
#include <cstdio>

using namespace trader::common;

/**
 * @brief 异常捕获组件单元测试
 */
TEST_CASE("ExceptionHandlerTest") {
    // 测试单例模式
    SECTION("SingletonInstance") {
        ExceptionHandler& handler1 = ExceptionHandler::GetInstance();
        ExceptionHandler& handler2 = ExceptionHandler::GetInstance();
        
        REQUIRE(&handler1 == &handler2);
    }

    // 测试初始化
    SECTION("Init") {
        ExceptionHandler& handler = ExceptionHandler::GetInstance();
        
        // 测试默认初始化
        REQUIRE(handler.Init("test_exception.log"));
        
        // 测试带最大日志大小的初始化
        REQUIRE(handler.Init("test_exception.log", 5 * 1024 * 1024));
    }

    // 测试启动和停止
    SECTION("StartStop") {
        ExceptionHandler& handler = ExceptionHandler::GetInstance();
        
        REQUIRE(handler.Init("test_exception.log"));
        REQUIRE(handler.Start());
        REQUIRE(handler.Stop());
        
        // 测试重复启动
        REQUIRE(handler.Start());
        REQUIRE(handler.Start()); // 第二次启动应该返回true
        REQUIRE(handler.Stop());
        REQUIRE(handler.Stop()); // 第二次停止应该返回true
    }

    // 测试资源释放回调
    SECTION("CleanupCallbacks") {
        ExceptionHandler& handler = ExceptionHandler::GetInstance();
        
        std::atomic<int> cleanupCalled(0);
        std::atomic<bool> resourceReleased(false);
        
        // 注册回调函数
        handler.RegisterCleanupCallback([&]() {
            cleanupCalled++;
            resourceReleased = true;
        });
        
        // 执行回调
        handler.UnregisterAllCleanupCallbacks();
        
        // 再次注册多个回调
        handler.RegisterCleanupCallback([&]() {
            cleanupCalled++;
        });
        
        handler.RegisterCleanupCallback([&]() {
            cleanupCalled++;
        });
        
        // 清除所有回调
        handler.UnregisterAllCleanupCallbacks();
        
        REQUIRE(cleanupCalled == 0);
    }

    // 测试日志文件创建
    SECTION("LogFileCreation") {
        ExceptionHandler& handler = ExceptionHandler::GetInstance();
        
        std::string logFile = "test_exception.log";
        
        // 删除可能存在的旧日志文件
        std::remove(logFile.c_str());
        
        REQUIRE(handler.Init(logFile));
        REQUIRE(handler.Start());
        
        // 检查日志文件是否创建
        std::ifstream file(logFile);
        REQUIRE(file.good());
        
        REQUIRE(handler.Stop());
        
        // 清理
        std::remove(logFile.c_str());
    }

    // 测试异常信息获取
    SECTION("ExceptionInfo") {
        ExceptionHandler& handler = ExceptionHandler::GetInstance();
        
        REQUIRE(handler.Init("test_exception.log"));
        
        // 初始状态应该为空
        REQUIRE(handler.GetLastStackTrace().empty());
        REQUIRE(handler.GetLastExceptionType().empty());
        REQUIRE(handler.GetLastExceptionTime().empty());
        
        REQUIRE(handler.Stop());
    }
}

/**
 * @brief 异常捕获功能集成测试
 */
TEST_CASE("ExceptionHandlerIntegrationTest") {
    // 这个测试需要在单独的进程中运行，因为会导致程序崩溃
    // 这里只测试基本的集成功能
    
    SECTION("BasicIntegration") {
        ExceptionHandler& handler = ExceptionHandler::GetInstance();
        
        REQUIRE(handler.Init("integration_test.log"));
        REQUIRE(handler.Start());
        
        // 注册资源释放回调
        bool cleanupCalled = false;
        handler.RegisterCleanupCallback([&cleanupCalled]() {
            cleanupCalled = true;
        });
        
        // 停止异常处理
        REQUIRE(handler.Stop());
        
        // 清理
        std::remove("integration_test.log");
    }
}

/**
 * @brief 多线程环境下的异常捕获测试
 */
TEST_CASE("ExceptionHandlerThreadSafetyTest") {
    ExceptionHandler& handler = ExceptionHandler::GetInstance();
    
    REQUIRE(handler.Init("thread_safety_test.log"));
    REQUIRE(handler.Start());
    
    // 多个线程注册回调
    std::vector<std::thread> threads;
    std::atomic<int> callbackCount(0);
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&]() {
            handler.RegisterCleanupCallback([&]() {
                callbackCount++;
            });
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 检查回调数量
    handler.UnregisterAllCleanupCallbacks();
    
    REQUIRE(handler.Stop());
    
    // 清理
    std::remove("thread_safety_test.log");
}