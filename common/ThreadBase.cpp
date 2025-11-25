#include "ThreadBase.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

ThreadBase::ThreadBase()
    : m_hThread(NULL)
    , m_dwThreadId(0)
    , m_bRunning(false)
    , m_bRunOnce(true)
{
    InitializeCriticalSection(&m_cs);
}

ThreadBase::~ThreadBase()
{
    Stop(5000); // 等待5秒让线程退出
    DeleteCriticalSection(&m_cs);
}

bool ThreadBase::Start(bool runOnce)
{
    EnterCriticalSection(&m_cs);
    if (m_bRunning)
    {
        LeaveCriticalSection(&m_cs);
        return false; // 线程已经在运行
    }

    m_bRunOnce = runOnce;
    m_bRunning = true;

    // 创建线程
    m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(
        NULL,           // 安全属性
        0,              // 栈大小
        ThreadEntry,    // 线程入口函数
        this,           // 传递给线程的参数
        0,              // 线程创建标志
        &m_dwThreadId   // 线程ID
    ));

    if (m_hThread == NULL)
    {
        m_bRunning = false;
        LeaveCriticalSection(&m_cs);
        return false;
    }

    LeaveCriticalSection(&m_cs);
    return true;
}

bool ThreadBase::Stop(unsigned int millisTime)
{
    EnterCriticalSection(&m_cs);
    if (!m_bRunning || m_hThread == NULL)
    {
        LeaveCriticalSection(&m_cs);
        return true; // 线程已经停止
    }

    m_bRunning = false;
    LeaveCriticalSection(&m_cs);

    // 等待线程退出
    DWORD dwWaitResult = WaitForSingleObject(m_hThread, millisTime);
    if (dwWaitResult == WAIT_OBJECT_0)
    {
        // 线程正常退出
        CloseHandle(m_hThread);
        m_hThread = NULL;
        m_dwThreadId = 0;
        return true;
    }
    else
    {
        // 线程超时未退出，强制终止
        if (TerminateThread(m_hThread, 0))
        {
            CloseHandle(m_hThread);
            m_hThread = NULL;
            m_dwThreadId = 0;

            // 修复Windows平台强杀线程后标准输出锁无法释放的问题
            // 强制刷新所有标准输出流
            fflush(stdout);
            fflush(stderr);

            // 重置标准输出流的缓冲区
            setvbuf(stdout, NULL, _IONBF, 0);
            setvbuf(stderr, NULL, _IONBF, 0);

            return true;
        }
        else
        {
            // 强制终止失败
            CloseHandle(m_hThread);
            m_hThread = NULL;
            m_dwThreadId = 0;
            return false;
        }
    }
}

bool ThreadBase::SetAffinity(const std::vector<DWORD>& cpuCores)
{
    EnterCriticalSection(&m_cs);
    if (!m_bRunning || m_hThread == NULL)
    {
        LeaveCriticalSection(&m_cs);
        return false; // 线程未运行
    }
    LeaveCriticalSection(&m_cs);

    if (cpuCores.empty())
    {
        return false; // CPU核列表为空
    }

    // 构建CPU亲和性掩码
    DWORD_PTR dwAffinityMask = 0;
    for (DWORD core : cpuCores)
    {
        if (core >= sizeof(DWORD_PTR) * 8)
        {
            return false; // CPU核编号超出范围
        }
        dwAffinityMask |= (1ULL << core);
    }

    // 设置线程亲和性
    if (SetThreadAffinityMask(m_hThread, dwAffinityMask))
    {
        return true;
    }
    else
    {
        return false;
    }
}

DWORD ThreadBase::GetThreadId() const
{
    return m_dwThreadId;
}

bool ThreadBase::GetRunOnce() const
{
    return m_bRunOnce;
}


unsigned int __stdcall ThreadBase::ThreadEntry(void* pParam)
{
    ThreadBase* pThis = static_cast<ThreadBase*>(pParam);
    if (pThis == NULL)
    {
        return 0;
    }

    pThis->ThreadLoop();
    return 0;
}

void ThreadBase::ThreadLoop()
{
    try
    {
        if (m_bRunOnce)
        {
            // 只运行一次
            Run();
        }
        else
        {
            // 循环运行，直到m_bRunning为false
            while (m_bRunning)
            {
                Run();
            }
        }
    }
    catch (const std::exception& e)
    {
        // 捕获并输出异常信息
        std::ostringstream oss;
        oss << "Thread " << m_dwThreadId << " exception: " << e.what() << std::endl;
        OutputDebugStringA(oss.str().c_str());
    }
    catch (...)
    {
        // 捕获未知异常
        std::ostringstream oss;
        oss << "Thread " << m_dwThreadId << " unknown exception" << std::endl;
        OutputDebugStringA(oss.str().c_str());
    }

    // 线程退出时重置运行标志
    EnterCriticalSection(&m_cs);
    m_bRunning = false;
    LeaveCriticalSection(&m_cs);
}
