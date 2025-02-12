// Log.cpp
#include "Log.h"
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <iostream>
Log::~Log() {
    running_ = false;
    if (write_thread_ && write_thread_->joinable()) {
        write_thread_->join();
    }
}

Log& Log::Instance() {
    static Log instance;
    return instance;
}

Log::Log() = default;

void Log::Init(LogLevel level, const std::string& path, size_t max_size) {
    current_level_ = level;
    base_path_ = path;  // 设置路径

    max_file_size_ = max_size;
    running_ = true;
    write_thread_ = std::make_unique<std::thread>(&Log::AsyncWrite, this);
}

void Log::AsyncWrite() {
    std::string msg;
    while (running_) {
        while (log_queue_.Pop(msg, 100)) {
            std::lock_guard<std::mutex> lock(file_mutex_);
            RotateFile();
            log_file_ << msg << std::endl;
        }
    }

    // 写入剩余日志
    while (log_queue_.Pop(msg)) {
        RotateFile();
        log_file_ << msg << std::endl;
    }
}

void Log::RotateFile() {
    namespace fs = std::filesystem;
    if (!log_file_.is_open()) {
        CreateNewFile();
        return;
    }

    // 时间滚动（每日）
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    
    if (tm.tm_mday != current_day_) {
        log_file_.close();
        CreateNewFile();
        current_day_ = tm.tm_mday;
        return;
    }

    // 大小滚动
    try {
        if (fs::file_size(current_file_) >= max_file_size_) {
            log_file_.close();
            CreateNewFile();
        }
    } catch (...) {
        CreateNewFile();
    }
}

void Log::CreateNewFile() {
    namespace fs = std::filesystem;

    // 确保日志目录存在
    if (!fs::exists(base_path_)) {
        fs::create_directories(base_path_);
    }

    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);

    // 使用线程ID或连接ID来区分不同日志文件
    std::ostringstream filename;
    
    if (base_path_.back() != '/' && base_path_.back() != '\\') {
        base_path_ += "/";  
    }

    // 拼接完整路径
    filename << base_path_ 
             << "log_" << std::this_thread::get_id() 
             << "_" << std::put_time(&tm, "%Y%m%d")
             << ".log";
    
    current_file_ = filename.str();
    log_file_.open(current_file_, std::ios::app);
}

const char* Log::LevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        case LogLevel::FATAL:   return "FATAL";
        default:                return "UNKNOWN";
    }
}

void Log::Flush() {
    std::lock_guard<std::mutex> lock(file_mutex_);
    if (log_file_.is_open()) {
        log_file_.flush();
    }
}

void Log::SetLevel(LogLevel level) {
    current_level_ = level;
}

LogLevel Log::GetLevel() const {
    return current_level_;
}
