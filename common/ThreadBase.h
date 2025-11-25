#pragma once

#include <windows.h>
#include <process.h>
#include <vector>

class ThreadBase
{
public:
    ThreadBase();
    virtual ~ThreadBase();

    // 启动线程，runOnce为true时线程执行一次Run()后退出，否则循环执行
    bool Start(bool runOnce = true);

    // 停止线程，millisTime为等待线程退出的毫秒数，超时后强制终止
    bool Stop(unsigned int millisTime);

    // 线程执行体，子类必须实现
    virtual void Run() = 0;

    // 设置线程亲和性，绑定到指定的CPU核
    bool SetAffinity(const std::vector<DWORD>& cpuCores);

    // 获取当前线程ID
    DWORD GetThreadId() const;

    // 获取是否只运行一次
    bool GetRunOnce() const;


private:
    // 线程入口函数
    static unsigned int __stdcall ThreadEntry(void* pParam);

    // 线程执行循环
    void ThreadLoop();

private:
    HANDLE m_hThread;          // 线程句柄
    DWORD m_dwThreadId;        // 线程ID
    volatile bool m_bRunning;  // 线程运行标志
    bool m_bRunOnce;           // 是否只运行一次
    CRITICAL_SECTION m_cs;     // 临界区，用于保护共享数据
};
