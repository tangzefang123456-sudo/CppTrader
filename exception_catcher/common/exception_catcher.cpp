#include "exception_catcher.h"
#include <iostream>
#include <fstream>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <execinfo.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

namespace Common {

std::string ExceptionCatcher::s_logFilePath;
bool ExceptionCatcher::s_isInitialized = false;
bool ExceptionCatcher::s_isStarted = false;
std::function<void()> ExceptionCatcher::s_releaseCallback;

ExceptionCatcher::ExceptionCatcher() {
}

ExceptionCatcher::~ExceptionCatcher() {
    if (s_isStarted) {
        Stop();
    }
}

bool ExceptionCatcher::Init(const std::string& logFilePath) {
    if (s_isInitialized) {
        std::cerr << "ExceptionCatcher is already initialized!" << std::endl;
        return false;
    }

    s_logFilePath = logFilePath;
    s_isInitialized = true;

    std::cout << "ExceptionCatcher initialized successfully! Log file: " << logFilePath << std::endl;
    return true;
}

bool ExceptionCatcher::Start() {
    if (!s_isInitialized) {
        std::cerr << "ExceptionCatcher is not initialized!" << std::endl;
        return false;
    }

    if (s_isStarted) {
        std::cerr << "ExceptionCatcher is already started!" << std::endl;
        return false;
    }

    // 设置信号处理函数
    signal(SIGSEGV, SignalHandler);
    signal(SIGABRT, SignalHandler);
    signal(SIGFPE, SignalHandler);
    signal(SIGILL, SignalHandler);
    signal(SIGTERM, SignalHandler);

    s_isStarted = true;

    std::cout << "ExceptionCatcher started successfully!" << std::endl;
    return true;
}

bool ExceptionCatcher::Stop() {
    if (!s_isStarted) {
        std::cerr << "ExceptionCatcher is not started!" << std::endl;
        return false;
    }

    // 恢复默认信号处理函数
    signal(SIGSEGV, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGTERM, SIG_DFL);

    s_isStarted = false;

    std::cout << "ExceptionCatcher stopped successfully!" << std::endl;
    return true;
}

void ExceptionCatcher::SetResourceReleaseCallback(std::function<void()> callback) {
    s_releaseCallback = callback;
}

void ExceptionCatcher::SignalHandler(int signal) {
    std::cerr << "Caught signal: " << signal << std::endl;

    // 写入堆栈信息到日志文件
    WriteStackTraceToLog(signal);

    // 释放所有资源
    ReleaseResources();

    // 退出程序
    exit(signal);
}

void ExceptionCatcher::WriteStackTraceToLog(int signal) {
    const int MAX_STACK_FRAMES = 64;
    void* stackFrames[MAX_STACK_FRAMES];
    int frameCount = backtrace(stackFrames, MAX_STACK_FRAMES);
    char** stackSymbols = backtrace_symbols(stackFrames, frameCount);

    // 打开日志文件
    std::ofstream logFile(s_logFilePath, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << s_logFilePath << std::endl;
        return;
    }

    // 写入日志头部信息
    time_t currentTime = time(nullptr);
    logFile << "==========================================" << std::endl;
    logFile << "Exception caught at: " << ctime(&currentTime);
    logFile << "Signal: " << signal << " (" << strsignal(signal) << ")" << std::endl;
    logFile << "Stack trace: " << std::endl;

    // 写入堆栈信息
    for (int i = 0; i < frameCount; ++i) {
        logFile << "  " << i << ": " << stackSymbols[i] << std::endl;
    }

    logFile << "==========================================" << std::endl;

    // 关闭日志文件
    logFile.close();

    // 释放堆栈符号
    free(stackSymbols);
}

void ExceptionCatcher::ReleaseResources() {
    std::cout << "Releasing resources..." << std::endl;

    // 调用资源释放回调函数
    if (s_releaseCallback) {
        try {
            s_releaseCallback();
        } catch (...) {
            std::cerr << "Exception occurred during resource release!" << std::endl;
        }
    }

    std::cout << "Resources released successfully!" << std::endl;
}

} // namespace Common
