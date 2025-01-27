#include "core/thread/taskScheduler.h"

namespace dxray
{
    TaskScheduler::TaskScheduler(const u16 a_numNoneOccupiedCores /*= 2*/) :
        m_currentLabel(1ul)
    {
        m_finishedLabel.store(1ul);
        m_workerCount = std::max(1u, std::thread::hardware_concurrency() - a_numNoneOccupiedCores);

        for (u16 i = 0; i < m_workerCount; ++i)
        {
            std::thread worker(&TaskScheduler::TaskScheduler::WorkerThread, this);
            worker.detach();
        }
    }

    TaskScheduler::~TaskScheduler()
    {
        Flush();
    }

    void TaskScheduler::Execute(const Task& a_treadJob)
    {
        m_currentLabel++;

        while (!m_taskPool.PushBack(a_treadJob))
        {
            Poll();
        }
    }

    bool TaskScheduler::IsBusy() const
    {
        return m_finishedLabel.load() < m_currentLabel;
    }

    void TaskScheduler::Wait()
    {
        while (IsBusy())
        {
            Poll();
        }
    }

    void TaskScheduler::Flush()
    {
        if (IsBusy())
        {
            Wait();
        }
    }

    void TaskScheduler::Poll()
    {
        m_wakeCondition.notify_one();
        std::this_thread::yield();
    }

    void TaskScheduler::WorkerThread()
    {
        Task task;
        while (true)
        {
            if (m_taskPool.PopFront(task))
            {
                task();
                m_finishedLabel.fetch_add(1ul);
                continue;
            }

            std::unique_lock<std::mutex> lock(m_lockMutex);
            m_wakeCondition.wait(lock);
        }
    }
}