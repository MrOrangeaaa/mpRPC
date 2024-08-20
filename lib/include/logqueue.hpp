#pragma once

#include <queue>
#include <thread>
#include <mutex>  // C++11对pthread_mutex_t的封装
#include <condition_variable>  // C++11对pthread_condition_t的封装

// 异步日志系统的缓冲队列
template<typename T>
class LogQueue
{
public:
    // 多个worker线程都会写logqueue
    void Push(const T &data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        m_cond.notify_one();
    }

    // 仅有一个线程读logqueue
    T Pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            // 日志队列为空，线程进入wait状态
            m_cond.wait(lock);
        }

        T data = m_queue.front();
        m_queue.pop();
        return data;
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cond;
};