#include "trader/common/ExceptionHandler.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <execinfo.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <direct.h>
#define mkdir _mkdir
#define unlink _unlink
#define rename _rename
#else
#include <sys/types.h>
#include <sys/wait.h>
#endif

namespace Trader {
namespace Common {

ExceptionHandler& ExceptionHandler::GetInstance() {
    static ExceptionHandler instance;
    return instance;
}

ExceptionHandler::ExceptionHandler()
    : m_initialized(false)
    , m_running(false)
    , m_logFilePath("exception.log") {
}

ExceptionHandler::~ExceptionHandler() {
    Stop();
}

bool ExceptionHandler::Init(const std::string& logFilePath) {
    if (m_initialized) {
        std::cerr << "ExceptionHandler already initialized" << std::endl;
        return false;
    }

    m_logFilePath = logFilePath;
    m_initialized = true;

    return true;
}

bool ExceptionHandler::Start() {
    if (!m_initialized) {
        std::cerr << "ExceptionHandler not initialized" << std::endl;
        return false;
    }

    if (m_running) {
        std::cerr << "ExceptionHandler already running" << std::endl;
        return false;
    }

    // 注册信号处理函数
    signal(SIGSEGV, SignalHandler);
    signal(SIGABRT, SignalHandler);
    signal(SIGFPE, SignalHandler);
    signal(SIGILL, SignalHandler);
    signal(SIGTERM, SignalHandler);

    m_running = true;

    return true;
}

bool ExceptionHandler::Stop() {
    if (!m_running) {
        return true;
    }

    // 恢复默认信号处理函数
    signal(SIGSEGV, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGTERM, SIG_DFL);

    m_running = false;

    return true;
}

void ExceptionHandler::RegisterCleanupCallback(CleanupCallback callback) {
    m_cleanupCallbacks.push_back(callback);
}

void ExceptionHandler::UnregisterCleanupCallback(CleanupCallback callback) {
    auto it = std::remove(m_cleanupCallbacks.begin(), m_cleanupCallbacks.end(), callback);
    m_cleanupCallbacks.erase(it, m_cleanupCallbacks.end());
}

void ExceptionHandler::SignalHandler(int signal) {
    ExceptionHandler& instance = GetInstance();
    instance.WriteExceptionLog(signal);
    instance.ExecuteCleanupCallbacks();
    exit(signal);
}

void ExceptionHandler::WriteExceptionLog(int signal) {
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm_info = std::localtime(&now_time);

    // 创建日志文件目录
    size_t pos = m_logFilePath.find_last_of("/\");
    if (pos != std::string::npos) {
        std::string dir = m_logFilePath.substr(0, pos);
#ifdef _WIN32
        _mkdir(dir.c_str());
#else
        mkdir(dir.c_str(), 0755);
#endif
    }

    // 打开日志文件
    std::ofstream log_file(m_logFilePath, std::ios::app);
    if (!log_file.is_open()) {
        std::cerr << "Failed to open exception log file: " << m_logFilePath << std::endl;
        return;
    }

    // 写入异常头部信息
    char time_buf[80];
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);
    log_file << "==========================================" << std::endl;
    log_file << "Exception occurred at: " << time_buf << std::endl;
    log_file << "Signal: " << signal << " (";

    switch (signal) {
        case SIGSEGV:
            log_file << "Segmentation Fault");
            break;
        case SIGABRT:
            log_file << "Abort");
            break;
        case SIGFPE:
            log_file << "Floating Point Exception");
            break;
        case SIGILL:
            log_file << "Illegal Instruction");
            break;
        case SIGTERM:
            log_file << "Termination Signal");
            break;
        default:
            log_file << "Unknown Signal");
            break;
    }

    log_file << std::endl;

    // 写入进程和线程ID
    log_file << "Process ID: " << getpid() << std::endl;
#ifdef _WIN32
    log_file << "Thread ID: " << GetCurrentThreadId() << std::endl;
#else
    log_file << "Thread ID: " << pthread_self() << std::endl;
#endif

    // 写入堆栈跟踪信息
    log_file << "Stack Trace:" << std::endl;
    log_file << GetStackTrace() << std::endl;

    // 关闭日志文件
    log_file.close();
}

void ExceptionHandler::ExecuteCleanupCallbacks() {
    for (auto& callback : m_cleanupCallbacks) {
        try {
            callback();
        } catch (const std::exception& e) {
            std::cerr << "Error executing cleanup callback: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown error executing cleanup callback" << std::endl;
        }
    }
}

std::string ExceptionHandler::GetStackTrace() {
    const int max_frames = 64;
    void* frames[max_frames];
    int num_frames = backtrace(frames, max_frames);
    char** symbols = backtrace_symbols(frames, num_frames);

    std::stringstream ss;

    if (symbols != nullptr) {
        for (int i = 1; i < num_frames; ++i) {  // 跳过第0帧（当前函数）
            ss << "  " << i << ": " << symbols[i] << std::endl;
        }
        free(symbols);
    } else {
        ss << "  Failed to get stack trace symbols" << std::endl;
    }

    return ss.str();
}

}  // namespace Common
}  // namespace Trader
