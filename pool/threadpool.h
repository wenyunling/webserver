#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <stdexcept>
#include "../pool/sql_pool.h"

template <typename T>
class threadpool
{
public:
    /* 构造函数：初始化线程池 */
    threadpool(int actor_model, connection_pool *connPool, int thread_number = 8, int max_requests = 10000);
    ~threadpool();

    /* 添加任务到队列 */
    bool append(T *request, int state);
    bool append_p(T *request);

private:
    /* 工作线程函数 */
    void worker();
    void run();

private:
    int m_thread_number;                        // 线程池中的线程数
    int m_max_requests;                         // 请求队列中允许的最大请求数
    std::vector<std::thread> m_threads;         // 线程数组
    std::list<T *> m_workqueue;                 // 请求队列
    std::mutex m_queuelocker;                   // 保护请求队列的互斥锁
    std::condition_variable m_queuestat;        // 条件变量，用于通知线程有新任务
    connection_pool *m_connPool;                // 数据库连接池
    int m_actor_model;                          // 模型切换
    std::atomic<bool> m_stop{false};            // 线程池停止标志
};

/* 构造函数 */
template <typename T>
threadpool<T>::threadpool(int actor_model, connection_pool *connPool, int thread_number, int max_requests)
    : m_actor_model(actor_model), m_thread_number(thread_number), m_max_requests(max_requests), m_connPool(connPool)
{
    if (thread_number <= 0 || max_requests <= 0)
        throw std::invalid_argument("Invalid thread number or max requests");

    // 创建线程
    for (int i = 0; i < thread_number; ++i)
    {
        m_threads.emplace_back(&threadpool<T>::worker, this);
    }
}

/* 析构函数 */
template <typename T>
threadpool<T>::~threadpool()
{
    m_stop = true; // 设置停止标志
    m_queuestat.notify_all(); // 唤醒所有线程

    // 等待所有线程结束
    for (auto &thread : m_threads)
    {
        if (thread.joinable())
            thread.join();
    }
}

/* 添加任务到队列（带状态） */
template <typename T>
bool threadpool<T>::append(T *request, int state)
{
    std::unique_lock<std::mutex> lock(m_queuelocker);
    if (m_workqueue.size() >= m_max_requests)
    {
        return false;
    }
    request->m_state = state;
    m_workqueue.push_back(request);
    lock.unlock();
    m_queuestat.notify_one(); // 通知一个等待的线程
    return true;
}

/* 添加任务到队列（不带状态） */
template <typename T>
bool threadpool<T>::append_p(T *request)
{
    std::unique_lock<std::mutex> lock(m_queuelocker);
    if (m_workqueue.size() >= m_max_requests)
    {
        return false;
    }
    m_workqueue.push_back(request);
    lock.unlock();
    m_queuestat.notify_one(); // 通知一个等待的线程
    return true;
}

/* 工作线程函数 */
template <typename T>
void threadpool<T>::worker()
{
    while (!m_stop)
    {
        run();
    }
}

/* 任务处理函数 */
template <typename T>
void threadpool<T>::run()
{
    std::unique_lock<std::mutex> lock(m_queuelocker);
    m_queuestat.wait(lock, [this] { return m_stop || !m_workqueue.empty(); });

    if (m_stop && m_workqueue.empty())
        return;

    T *request = m_workqueue.front();
    m_workqueue.pop_front();
    lock.unlock();

    if (!request)
        return;

    if (1 == m_actor_model)
    {
        if (0 == request->m_state)
        {
            if (request->read_once())
            {
                request->improv = 1;
                connectionRAII mysqlcon(&request->mysql, m_connPool);
                request->process();
            }
            else
            {
                request->improv = 1;
                request->timer_flag = 1;
            }
        }
        else
        {
            if (request->write())
            {
                request->improv = 1;
            }
            else
            {
                request->improv = 1;
                request->timer_flag = 1;
            }
        }
    }
    else
    {
        connectionRAII mysqlcon(&request->mysql, m_connPool);
        request->process();
    }
}

#endif