#pragma once

#include <string>
#include <functional>
#include <vector>

namespace Trader {
namespace Common {

/**
 * @brief C++异常捕获基础组件
 * 
 * 该组件用于捕获C++程序中的段错误等异常，并将堆栈信息写入文件日志中，供开发者分析原因。
 * 组件提供Init、Start、Stop接口，支持注册资源释放回调函数，在截获异常后退出程序之前释放所有资源。
 */
class ExceptionHandler {
public:
    /**
     * @brief 资源释放回调函数类型
     */
    using CleanupCallback = std::function<void()>;

    /**
     * @brief 获取单例实例
     * 
     * @return ExceptionHandler& 单例实例引用
     */
    static ExceptionHandler& GetInstance();

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
     * @brief 注册资源释放回调函数
     * 
     * @param callback 资源释放回调函数
     */
    void RegisterCleanupCallback(CleanupCallback callback);

    /**
     * @brief 移除资源释放回调函数
     * 
     * @param callback 资源释放回调函数
     */
    void UnregisterCleanupCallback(CleanupCallback callback);

private:
    /**
     * @brief 私有构造函数，防止外部实例化
     */
    ExceptionHandler();

    /**
     * @brief 私有析构函数，防止外部删除
     */
    ~ExceptionHandler();

    /**
     * @brief 异常处理函数
     * 
     * @param signal 信号类型
     */
    static void SignalHandler(int signal);

    /**
     * @brief 写入异常日志
     * 
     * @param signal 信号类型
     */
    void WriteExceptionLog(int signal);

    /**
     * @brief 执行资源释放回调函数
     */
    void ExecuteCleanupCallbacks();

    /**
     * @brief 获取堆栈跟踪信息
     * 
     * @return std::string 堆栈跟踪信息
     */
    std::string GetStackTrace();

private:
    bool m_initialized;  ///< 组件是否已初始化
    bool m_running;      ///< 组件是否正在运行
    std::string m_logFilePath;  ///< 日志文件路径
    std::vector<CleanupCallback> m_cleanupCallbacks;  ///< 资源释放回调函数列表
};

}  // namespace Common
}  // namespace Trader
