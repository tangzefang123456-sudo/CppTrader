#include "gtest/gtest.h"
#include "ThreadBase.h"
#include <iostream>
#include <chrono>
#include <atomic>

// 测试线程类，继承自ThreadBase
class TestThread : public ThreadBase
{
public:
    TestThread() : m_nRunCount(0), m_bShouldExit(false) {}

    virtual void Run() override
    {
        m_nRunCount++;
        std::cout << "TestThread::Run() called, count: " << m_nRunCount << std::endl;

        // 如果是循环运行模式，等待一段时间再退出
        if (!GetRunOnce())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    int GetRunCount() const { return m_nRunCount; }
    void SetShouldExit(bool bExit) { m_bShouldExit = bExit; }

private:
    std::atomic<int> m_nRunCount; // 运行次数
    bool m_bShouldExit;           // 是否应该退出
};

// 测试线程启动和停止
TEST(ThreadBaseTest, StartStop)
{
    TestThread thread;

    // 测试启动线程
    EXPECT_TRUE(thread.Start(true));
    EXPECT_FALSE(thread.Start(true)); // 重复启动应该失败

    // 等待线程执行完成
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 测试停止线程
    EXPECT_TRUE(thread.Stop(1000));
    EXPECT_TRUE(thread.Stop(1000)); // 重复停止应该成功

    // 检查线程是否只运行了一次
    EXPECT_EQ(thread.GetRunCount(), 1);
}

// 测试循环运行模式
TEST(ThreadBaseTest, RunLoop)
{
    TestThread thread;

    // 启动循环运行模式
    EXPECT_TRUE(thread.Start(false));

    // 等待线程运行几次
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 停止线程
    EXPECT_TRUE(thread.Stop(1000));

    // 检查线程是否运行了多次
    EXPECT_GT(thread.GetRunCount(), 1);
}

// 测试CPU亲和性设置
TEST(ThreadBaseTest, SetAffinity)
{
    TestThread thread;

    // 启动线程
    EXPECT_TRUE(thread.Start(true));

    // 设置CPU亲和性到第0核
    std::vector<DWORD> cores = { 0 };
    EXPECT_TRUE(thread.SetAffinity(cores));

    // 停止线程
    EXPECT_TRUE(thread.Stop(1000));
}

// 主函数
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
