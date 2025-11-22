#ifndef TRADER_COMMON_EXCEPTIONHANDLER_H
#define TRADER_COMMON_EXCEPTIONHANDLER_H

#include <string>
#include <vector>
#include <functional>

namespace trader {
namespace common {

/**
 * @brief 异常捕获基础组件
 * 
 * 该组件用于捕获C++程序中的段错误等异常，并生成详细的堆栈信息写入日志文件。
 * 提供Init、Start、Stop接口，支持注册资源释放回调函数，确保异常发生后能正确释放所有资源。
 */
class ExceptionHandler {
public:
    /**
     * @brief 资源释放回调函数类型
     */
    using CleanupCallback = std::function<void()>;

    /**
     * @brief 获取异常处理器的单例实例
     * 
     * @return ExceptionHandler& 异常处理器实例
     */
    static ExceptionHandler& GetInstance();

    /**
     * @brief 初始化异常处理器
     * 
     * @param logFilePath 日志文件路径
     * @param maxLogSize 最大日志文件大小（字节），默认10MB
     * @return bool 初始化成功返回true，失败返回false
     */
    bool Init(const std::string& logFilePath, size_t maxLogSize = 10 * 1024 * 1024);

    /**
     * @brief 启动异常捕获
     * 
     * 注册信号处理函数，开始捕获异常
     * @return bool 启动成功返回true，失败返回false
     */
    bool Start();

    /**
     * @brief 停止异常捕获
     * 
     * 取消注册信号处理函数，停止捕获异常
     * @return bool 停止成功返回true，失败返回false
     */
    bool Stop();

    /**
     * @brief 注册资源释放回调函数
     * 
     * 当异常发生时，所有注册的回调函数都会被调用，用于释放资源
     * @param callback 资源释放回调函数
     */
    void RegisterCleanupCallback(CleanupCallback callback);

    /**
     * @brief 移除所有注册的资源释放回调函数
     */
    void UnregisterAllCleanupCallbacks();

    /**
     * @brief 获取最后一次异常的堆栈信息
     * 
     * @return const std::vector<std::string>& 堆栈信息列表
     */
    const std::vector<std::string>& GetLastStackTrace() const;

    /**
     * @brief 获取最后一次异常的类型
     * 
     * @return const std::string& 异常类型
     */
    const std::string& GetLastExceptionType() const;

    /**
     * @brief 获取最后一次异常发生的时间
     * 
     * @return const std::string& 异常时间字符串
     */
    const std::string& GetLastExceptionTime() const;

    /**
     * @brief 禁用拷贝构造函数
     */
    ExceptionHandler(const ExceptionHandler&) = delete;

    /**
     * @brief 禁用赋值运算符
     */
    ExceptionHandler& operator=(const ExceptionHandler&) = delete;

private:
    /**
     * @brief 构造函数
     */
    ExceptionHandler();

    /**
     * @brief 析构函数
     */
    ~ExceptionHandler();

    /**
     * @brief 信号处理函数
     * 
     * @param signal 信号类型
     */
    static void SignalHandler(int signal);

    /**
     * @brief 生成堆栈跟踪信息
     * 
     * @param stackTrace 输出堆栈信息列表
     * @return bool 生成成功返回true，失败返回false
     */
    bool GenerateStackTrace(std::vector<std::string>& stackTrace);

    /**
     * @brief 写入异常信息到日志文件
     * 
     * @param signal 信号类型
     * @param stackTrace 堆栈信息
     */
    void WriteExceptionLog(int signal, const std::vector<std::string>& stackTrace);

    /**
     * @brief 执行所有资源释放回调函数
     */
    void ExecuteCleanupCallbacks();

    /**
     * @brief 获取当前时间字符串
     * 
     * @return std::string 当前时间字符串
     */
    std::string GetCurrentTimeString();

    /**
     * @brief 检查并滚动日志文件
     * 
     * @return bool 滚动成功返回true，失败返回false
     */
    bool CheckAndRollLogFile();

private:
    std::string logFilePath_;          ///< 日志文件路径
    size_t maxLogSize_;                ///< 最大日志文件大小
    bool isStarted_;                   ///< 异常捕获是否已启动
    std::vector<CleanupCallback> cleanupCallbacks_; ///< 资源释放回调函数列表
    
    // 最后一次异常信息
    std::vector<std::string> lastStackTrace_;
    std::string lastExceptionType_;
    std::string lastExceptionTime_;
};

} // namespace common
} // namespace trader

#endif // TRADER_COMMON_EXCEPTIONHANDLER_H