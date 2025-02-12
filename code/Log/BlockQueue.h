// BlockQueue.h
#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class BlockQueue {
public:
    void Push(const T& item) {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push(item);
        cond_.notify_one();
    }

    bool Pop(T& item, int timeout_ms = 0) {
        std::unique_lock<std::mutex> lock(mtx_);
        if (timeout_ms > 0) {
            auto status = cond_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                                        [this] { return !queue_.empty(); });
            if (!status) return false;
        } else {
            cond_.wait(lock, [this] { return !queue_.empty(); });
        }
        
        item = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    size_t Size() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.size();
    }

private:
    std::queue<T> queue_;
    mutable std::mutex mtx_;
    std::condition_variable cond_;
};
