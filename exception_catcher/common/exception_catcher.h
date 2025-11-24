#ifndef EXCEPTION_CATCHER_H
#define EXCEPTION_CATCHER_H

#include <string>
#include <functional>

namespace Common {

/**
 * @brief 异常捕获基础组件
 * 
 * 该组件用于捕获C++程序中的段错误等异常，并将堆栈信息写入文件日志中。
 * 组件提供Init、Start、Stop接口，支持资源的初始化、启动和停止。
 * 当截获到异常后，组件会在退出程序之前释放所有工作线程占用的资源。
 */
class ExceptionCatcher {
public:
    /**
     * @brief 构造函数
     */
    ExceptionCatcher();

    /**
     * @brief 析构函数
     */
    ~ExceptionCatcher();

    /**
     * @brief 初始化异常捕获组件
     * 
     * @param logFilePath 日志文件路径
     * @return bool 初始化成功返回true，否则返回false
     */
    bool Init(const std::string& logFilePath);

    /**
     * @brief 启动异常捕获组件
     * 
     * @return bool 启动成功返回true，否则返回false
     */
    bool Start();

    /**
     * @brief 停止异常捕获组件
     * 
     * @return bool 停止成功返回true，否则返回false
     */
    bool Stop();

    /**
     * @brief 设置资源释放回调函数
     * 
     * @param callback 资源释放回调函数
     */
    void SetResourceReleaseCallback(std::function<void()> callback);

private:
    /**
     * @brief 异常处理函数
     * 
     * @param signal 捕获到的信号
     */
    static void SignalHandler(int signal);

    /**
     * @brief 写入堆栈信息到日志文件
     * 
     * @param signal 捕获到的信号
     */
    static void WriteStackTraceToLog(int signal);

    /**
     * @brief 释放所有资源
     */
    static void ReleaseResources();

private:
    static std::string s_logFilePath;          // 日志文件路径
    static bool s_isInitialized;               // 组件是否已初始化
    static bool s_isStarted;                   // 组件是否已启动
    static std::function<void()> s_releaseCallback; // 资源释放回调函数
};

} // namespace Common

#endif // EXCEPTION_CATCHER_H
