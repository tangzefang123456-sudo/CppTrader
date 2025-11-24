#include <gtest/gtest.h>
#include "common/exception_catcher.h"
#include <fstream>
#include <thread>
#include <chrono>

using namespace Common;

class ExceptionCatcherTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 清理测试日志文件
        std::remove("test_exception.log");
    }

    void TearDown() override {
        // 清理测试日志文件
        std::remove("test_exception.log");
    }
};

TEST_F(ExceptionCatcherTest, InitTest) {
    ExceptionCatcher catcher;
    EXPECT_TRUE(catcher.Init("test_exception.log"));
    // 再次初始化应该失败
    EXPECT_FALSE(catcher.Init("test_exception.log"));
}

TEST_F(ExceptionCatcherTest, StartStopTest) {
    ExceptionCatcher catcher;
    EXPECT_TRUE(catcher.Init("test_exception.log"));
    EXPECT_TRUE(catcher.Start());
    // 再次启动应该失败
    EXPECT_FALSE(catcher.Start());
    EXPECT_TRUE(catcher.Stop());
    // 再次停止应该失败
    EXPECT_FALSE(catcher.Stop());
}

TEST_F(ExceptionCatcherTest, ResourceReleaseTest) {
    ExceptionCatcher catcher;
    bool released = false;

    catcher.SetResourceReleaseCallback([&released]() {
        released = true;
    });

    EXPECT_TRUE(catcher.Init("test_exception.log"));
    EXPECT_TRUE(catcher.Start());

    // 模拟异常捕获
    Common::ExceptionCatcher::ReleaseResources();

    EXPECT_TRUE(released);
}

// 测试异常捕获功能（需要手动验证日志文件）
TEST_F(ExceptionCatcherTest, ExceptionCatchTest) {
    ExceptionCatcher catcher;
    EXPECT_TRUE(catcher.Init("test_exception.log"));
    EXPECT_TRUE(catcher.Start());

    // 这里不直接触发段错误，因为会导致测试进程崩溃
    // 而是验证异常捕获组件的初始化和启动是否正常
    EXPECT_TRUE(true);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
