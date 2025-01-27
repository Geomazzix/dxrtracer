#pragma once
#include <queue>
#include <atomic>
#include <condition_variable>
#include <functional>

#include "core/valueTypes.h"

namespace dxray
{
    /// <summary>
    /// Primitive task scheduler, executes tasks when provided.
    /// #Note: Implementation taken from: https://wickedengine.net/2018/11/simple-job-system-using-standard-c/
    /// Incredibly basic TaskScheduler, should investigate in better job systems if needed in the future. This one is mainly used for the CPU raytracer 
    /// to boost trace times.
    /// </summary>
    class TaskScheduler final
    {
    public:
        using Task = std::function<void()>;
        static const usize MaxNumQueuedTasks = 256;

    private:
        /// <summary>
        /// Thread safe ring buffer, used as jobpool, serves on first comes first served.
        /// </summary>
        /// <typeparam name="T">ElementType</typeparam>
        /// <typeparam name="capacity"></typeparam>
        template<typename T, usize capacity>
        class ThreadSafeRingBuffer final
        {
        public:
            using ElementType = T;

            ThreadSafeRingBuffer();
            ~ThreadSafeRingBuffer() = default;

            bool PushBack(const T& item);
            bool PopFront(T& item);

            usize Getcapacity() const;

        private:
            usize m_head;
            usize m_tail;
            T m_data[capacity];
            std::mutex m_RingBufferMutex;
        };

        using TaskPool = ThreadSafeRingBuffer<Task, MaxNumQueuedTasks>;

    public:
        TaskScheduler(const u16 a_numNoneOccupiedCores = 2);
        ~TaskScheduler();

        void Execute(const Task& a_treadJob);
        bool IsBusy() const;
        void Wait();
        void Flush();
        usize GetWorkerCount() const;

    private:
        TaskPool m_taskPool;
        std::mutex m_lockMutex;
        std::condition_variable m_wakeCondition;
        std::atomic<u64> m_finishedLabel;
        usize m_workerCount;
        u64 m_currentLabel;

        void Poll();
        void WorkerThread();
    };

    template<typename T, usize capacity>
    TaskScheduler::ThreadSafeRingBuffer<T, capacity>::ThreadSafeRingBuffer() :
        m_head(0),
        m_tail(0)
    {
    }

    template<typename T, usize capacity>
    bool TaskScheduler::ThreadSafeRingBuffer<T, capacity>::PopFront(T& a_item)
    {
        const std::lock_guard<std::mutex> lock(m_RingBufferMutex);

        if (m_tail == m_head)
        {
            return false;
        }

        a_item = m_data[m_tail];
        m_tail = (m_tail + 1) % capacity;
        return true;
    }

    template<typename T, usize capacity>
    bool TaskScheduler::ThreadSafeRingBuffer<T, capacity>::PushBack(const T& a_item)
    {
        const std::lock_guard<std::mutex> lock(m_RingBufferMutex);

        usize next = (m_head + 1) % capacity;
        if (next == m_tail)
        {
            return false;
        }

        m_data[m_head] = a_item;
        m_head = next;
        return true;
    }

    template<typename T, usize capacity>
    usize TaskScheduler::ThreadSafeRingBuffer<T, capacity>::Getcapacity() const
    {
        return capacity;
    }

    inline usize TaskScheduler::GetWorkerCount() const
    {
        return m_workerCount;
    }
}