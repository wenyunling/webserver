// Log.h
#pragma once
#include "BlockQueue.h"
#include <atomic>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <thread>
#include <mutex>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

class Log {
public:
    ~Log();
    
    static Log& Instance();
    
    void Init(LogLevel level = LogLevel::INFO,
             const std::string& path = "./logs",
             size_t max_size = 1024 * 1024 * 10);

    template<typename... Args>
    void LogMessage(LogLevel level, const char* file, int line, Args&&... args);

    void Flush();
    void SetLevel(LogLevel level);
    LogLevel GetLevel() const;

private:
    Log();
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    void AsyncWrite();
    void RotateFile();
    void CreateNewFile();
    static const char* LevelToString(LogLevel level);

    std::atomic<bool> running_{false};
    std::unique_ptr<std::thread> write_thread_;
    BlockQueue<std::string> log_queue_;
    
    std::string base_path_;
    size_t max_file_size_;
    std::ofstream log_file_;
    mutable std::mutex file_mutex_;
    LogLevel current_level_ = LogLevel::INFO;
    
    std::string current_file_;
    int current_day_ = 0;
    std::atomic<int> seq_num_{1};
};

// 模板方法实现必须保留在头文件中
template<typename... Args>
void Log::LogMessage(LogLevel level, const char* file, int line, Args&&... args) {
    if (level < current_level_) return;

    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream oss;
    oss << "[" << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S") << "] "
        << "[" << LevelToString(level) << "] "
        << "[" << std::this_thread::get_id() << "] "
        << "[" << file << ":" << line << "] ";

    (oss << ... << std::forward<Args>(args));

    log_queue_.Push(oss.str());
}

// 日志宏定义
#define LOG(level, ...) Log::Instance().LogMessage(level, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) LOG(LogLevel::DEBUG, __VA_ARGS__)
#define LOG_INFO(...)  LOG(LogLevel::INFO, __VA_ARGS__)
#define LOG_WARN(...)  LOG(LogLevel::WARNING, __VA_ARGS__)
#define LOG_ERROR(...) LOG(LogLevel::ERROR, __VA_ARGS__)
#define LOG_FATAL(...) LOG(LogLevel::FATAL, __VA_ARGS__)
