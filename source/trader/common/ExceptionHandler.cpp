#include "trader/common/ExceptionHandler.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <signal.h>
#include <execinfo.h>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#endif

namespace trader {
namespace common {

// 全局变量用于保存异常处理器实例指针
static ExceptionHandler* g_exceptionHandlerInstance = nullptr;
static std::mutex g_instanceMutex;

ExceptionHandler::ExceptionHandler()
    : logFilePath_("exception.log"),
      maxLogSize_(10 * 1024 * 1024),
      isStarted_(false) {
}

ExceptionHandler::~ExceptionHandler() {
    Stop();
}

ExceptionHandler& ExceptionHandler::GetInstance() {
    std::lock_guard<std::mutex> lock(g_instanceMutex);
    if (g_exceptionHandlerInstance == nullptr) {
        g_exceptionHandlerInstance = new ExceptionHandler();
    }
    return *g_exceptionHandlerInstance;
}

bool ExceptionHandler::Init(const std::string& logFilePath, size_t maxLogSize) {
    if (isStarted_) {
        return false;
    }

    logFilePath_ = logFilePath;
    maxLogSize_ = maxLogSize;

    // 检查日志文件目录是否存在
    size_t lastSlash = logFilePath_.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        std::string directory = logFilePath_.substr(0, lastSlash);
#ifdef _WIN32
        if (_mkdir(directory.c_str()) != 0 && errno != EEXIST) {
#else
        if (mkdir(directory.c_str(), 0755) != 0 && errno != EEXIST) {
#endif
            return false;
        }
    }

    return true;
}

bool ExceptionHandler::Start() {
    if (isStarted_) {
        return true;
    }

    // 保存当前异常处理器实例指针
    g_exceptionHandlerInstance = this;

    // 注册信号处理函数
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SignalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    // 注册需要捕获的信号
    if (sigaction(SIGSEGV, &sa, nullptr) != 0) { return false; }
    if (sigaction(SIGILL, &sa, nullptr) != 0) { return false; }
    if (sigaction(SIGFPE, &sa, nullptr) != 0) { return false; }
    if (sigaction(SIGABRT, &sa, nullptr) != 0) { return false; }
    if (sigaction(SIGBUS, &sa, nullptr) != 0) { return false; }
    if (sigaction(SIGSYS, &sa, nullptr) != 0) { return false; }

    isStarted_ = true;
    return true;
}

bool ExceptionHandler::Stop() {
    if (!isStarted_) {
        return true;
    }

    // 恢复默认信号处理函数
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGSEGV, &sa, nullptr) != 0) { return false; }
    if (sigaction(SIGILL, &sa, nullptr) != 0) { return false; }
    if (sigaction(SIGFPE, &sa, nullptr) != 0) { return false; }
    if (sigaction(SIGABRT, &sa, nullptr) != 0) { return false; }
    if (sigaction(SIGBUS, &sa, nullptr) != 0) { return false; }
    if (sigaction(SIGSYS, &sa, nullptr) != 0) { return false; }

    isStarted_ = false;
    return true;
}

void ExceptionHandler::RegisterCleanupCallback(CleanupCallback callback) {
    if (callback) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        cleanupCallbacks_.push_back(callback);
    }
}

void ExceptionHandler::UnregisterAllCleanupCallbacks() {
    std::lock_guard<std::mutex> lock(g_instanceMutex);
    cleanupCallbacks_.clear();
}

const std::vector<std::string>& ExceptionHandler::GetLastStackTrace() const {
    return lastStackTrace_;
}

const std::string& ExceptionHandler::GetLastExceptionType() const {
    return lastExceptionType_;
}

const std::string& ExceptionHandler::GetLastExceptionTime() const {
    return lastExceptionTime_;
}

void ExceptionHandler::SignalHandler(int signal) {
    if (g_exceptionHandlerInstance == nullptr) {
        // 如果异常处理器未初始化，使用默认处理
        signal(signal, SIG_DFL);
        raise(signal);
        return;
    }

    // 生成堆栈跟踪
    std::vector<std::string> stackTrace;
    g_exceptionHandlerInstance->GenerateStackTrace(stackTrace);

    // 保存异常信息
    g_exceptionHandlerInstance->lastStackTrace_ = stackTrace;
    g_exceptionHandlerInstance->lastExceptionTime_ = g_exceptionHandlerInstance->GetCurrentTimeString();

    // 确定异常类型
    switch (signal) {
        case SIGSEGV: g_exceptionHandlerInstance->lastExceptionType_ = "Segmentation Fault (SIGSEGV)"; break;
        case SIGILL: g_exceptionHandlerInstance->lastExceptionType_ = "Illegal Instruction (SIGILL)"; break;
        case SIGFPE: g_exceptionHandlerInstance->lastExceptionType_ = "Floating Point Exception (SIGFPE)"; break;
        case SIGABRT: g_exceptionHandlerInstance->lastExceptionType_ = "Abort Signal (SIGABRT)"; break;
        case SIGBUS: g_exceptionHandlerInstance->lastExceptionType_ = "Bus Error (SIGBUS)"; break;
        case SIGSYS: g_exceptionHandlerInstance->lastExceptionType_ = "Bad System Call (SIGSYS)"; break;
        default: g_exceptionHandlerInstance->lastExceptionType_ = "Unknown Signal (" + std::to_string(signal) + ")"; break;
    }

    // 写入异常日志
    g_exceptionHandlerInstance->WriteExceptionLog(signal, stackTrace);

    // 执行资源释放回调
    g_exceptionHandlerInstance->ExecuteCleanupCallbacks();

    // 退出程序
    _exit(EXIT_FAILURE);
}

bool ExceptionHandler::GenerateStackTrace(std::vector<std::string>& stackTrace) {
    const int MAX_FRAMES = 64;
    void* frames[MAX_FRAMES];
    int frameCount = backtrace(frames, MAX_FRAMES);
    
    if (frameCount <= 0) {
        stackTrace.push_back("Failed to get stack trace");
        return false;
    }

    char** symbols = backtrace_symbols(frames, frameCount);
    if (symbols == nullptr) {
        stackTrace.push_back("Failed to get stack symbols");
        return false;
    }

    for (int i = 0; i < frameCount; ++i) {
        stackTrace.push_back(symbols[i]);
    }

    free(symbols);
    return true;
}

void ExceptionHandler::WriteExceptionLog(int signal, const std::vector<std::string>& stackTrace) {
    // 检查并滚动日志文件
    CheckAndRollLogFile();

    // 打开日志文件
    std::ofstream logFile(logFilePath_, std::ios::app);
    if (!logFile.is_open()) {
        return;
    }

    // 写入异常头部信息
    logFile << "===========================================" << std::endl;
    logFile << "Exception Time: " << GetCurrentTimeString() << std::endl;
    logFile << "Signal: " << signal << " - ";
    
    switch (signal) {
        case SIGSEGV: logFile << "Segmentation Fault (SIGSEGV)"; break;
        case SIGILL: logFile << "Illegal Instruction (SIGILL)"; break;
        case SIGFPE: logFile << "Floating Point Exception (SIGFPE)"; break;
        case SIGABRT: logFile << "Abort Signal (SIGABRT)"; break;
        case SIGBUS: logFile << "Bus Error (SIGBUS)"; break;
        case SIGSYS: logFile << "Bad System Call (SIGSYS)"; break;
        default: logFile << "Unknown Signal";
    }
    logFile << std::endl;

    // 写入进程信息
    logFile << "Process ID: " << getpid() << std::endl;
#ifdef _WIN32
    logFile << "Thread ID: " << GetCurrentThreadId() << std::endl;
#else
    logFile << "Thread ID: " << pthread_self() << std::endl;
#endif

    // 写入堆栈跟踪
    logFile << "Stack Trace:" << std::endl;
    for (size_t i = 0; i < stackTrace.size(); ++i) {
        logFile << "  " << i << ": " << stackTrace[i] << std::endl;
    }

    logFile << "===========================================" << std::endl << std::endl;

    logFile.close();
}

void ExceptionHandler::ExecuteCleanupCallbacks() {
    std::lock_guard<std::mutex> lock(g_instanceMutex);
    for (auto& callback : cleanupCallbacks_) {
        try {
            callback();
        } catch (...) {
            // 忽略回调中的异常
        }
    }
}

std::string ExceptionHandler::GetCurrentTimeString() {
    time_t now = time(nullptr);
    char buffer[80];
    struct tm timeInfo;

#ifdef _WIN32
    localtime_s(&timeInfo, &now);
#else
    localtime_r(&now, &timeInfo);
#endif

    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
    return std::string(buffer);
}

bool ExceptionHandler::CheckAndRollLogFile() {
    struct stat fileInfo;
    if (stat(logFilePath_.c_str(), &fileInfo) != 0) {
        // 文件不存在，不需要滚动
        return true;
    }

    if (fileInfo.st_size < maxLogSize_) {
        // 文件大小未超过限制
        return true;
    }

    // 滚动日志文件
    std::string backupFile = logFilePath_ + ".bak";
    
    // 删除旧的备份文件
#ifdef _WIN32
    _unlink(backupFile.c_str());
#else
    unlink(backupFile.c_str());
#endif

    // 重命名当前日志文件为备份文件
#ifdef _WIN32
    if (_rename(logFilePath_.c_str(), backupFile.c_str()) != 0) {
#else
    if (rename(logFilePath_.c_str(), backupFile.c_str()) != 0) {
#endif
        return false;
    }

    return true;
}

} // namespace common
} // namespace trader