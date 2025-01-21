#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <mutex>  // 引入 C++11 的互斥量
#include <condition_variable>  // 引入 C++11 的条件变量
using namespace std;

template <class T>
class block_queue
{
public:
    block_queue(int max_size = 1000)
    {
        if (max_size <= 0)
        {
            exit(-1);
        }

        m_max_size = max_size;
        m_array = new T[max_size];
        m_size = 0;
        m_front = -1;
        m_back = -1;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);  // 使用 std::lock_guard 加锁
        m_size = 0;
        m_front = -1;
        m_back = -1;
    }

    ~block_queue()
    {
        std::lock_guard<std::mutex> lock(m_mutex);  // 使用 std::lock_guard 加锁
        if (m_array != NULL)
            delete[] m_array;
    }

    // 判断队列是否满了
    bool full()
    {
        std::lock_guard<std::mutex> lock(m_mutex);  // 使用 std::lock_guard 加锁
        return m_size >= m_max_size;
    }

    // 判断队列是否为空
    bool empty()
    {
        std::lock_guard<std::mutex> lock(m_mutex);  // 使用 std::lock_guard 加锁
        return m_size == 0;
    }

    // 返回队首元素
    bool front(T &value)
    {
        std::lock_guard<std::mutex> lock(m_mutex);  // 使用 std::lock_guard 加锁
        if (m_size == 0)
        {
            return false;
        }
        value = m_array[m_front];
        return true;
    }

    // 返回队尾元素
    bool back(T &value)
    {
        std::lock_guard<std::mutex> lock(m_mutex);  // 使用 std::lock_guard 加锁
        if (m_size == 0)
        {
            return false;
        }
        value = m_array[m_back];
        return true;
    }

    int size()
    {
        std::lock_guard<std::mutex> lock(m_mutex);  // 使用 std::lock_guard 加锁
        return m_size;
    }

    int max_size()
    {
        std::lock_guard<std::mutex> lock(m_mutex);  // 使用 std::lock_guard 加锁
        return m_max_size;
    }

    // 往队列添加元素，需要将所有使用队列的线程先唤醒
    bool push(const T &item)
    {
        std::lock_guard<std::mutex> lock(m_mutex);  // 使用 std::lock_guard 加锁
        if (m_size >= m_max_size)
        {
            m_cond.notify_all();
            return false;
        }

        m_back = (m_back + 1) % m_max_size;
        m_array[m_back] = item;
        m_size++;

        m_cond.notify_all();
        return true;
    }

    // pop时,如果当前队列没有元素,将会等待条件变量
    bool pop(T &item)
    {
        std::unique_lock<std::mutex> lock(m_mutex);  // 使用 std::unique_lock 加锁
        while (m_size <= 0)
        {
            m_cond.wait(lock);
        }

        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        m_size--;
        return true;
    }

    // 增加了超时处理
    bool pop(T &item, int ms_timeout)
    {
        std::unique_lock<std::mutex> lock(m_mutex);  // 使用 std::unique_lock 加锁
        if (m_size <= 0)
        {
            if (m_cond.wait_for(lock, std::chrono::milliseconds(ms_timeout)) == std::cv_status::timeout)
            {
                return false;
            }
        }

        if (m_size <= 0)
        {
            return false;
        }

        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        m_size--;
        return true;
    }

private:
    std::mutex m_mutex;  // 使用 std::mutex 替代 locker
    std::condition_variable m_cond;  // 使用 std::condition_variable 替代 cond

    T *m_array;
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;
};

#endif