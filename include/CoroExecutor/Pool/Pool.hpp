#ifndef CORO_EXECUTOR_POOL_H
#define CORO_EXECUTOR_POOL_H

#include <queue>
#include <mutex>

namespace CoroExecutor
{


struct WorkItem
{
    int priority;
    std::coroutine_handle<> h;
};


template <typename T>
concept StorageStrategy = requires(T strat, WorkItem w, size_t thread_id) 
{
    strat.push(w, thread_id);
    {strat.pop(thread_id)} -> std::convertible_to<std::optional<WorkItem>>;
};

template<typename T>
concept ScheduleStrategy = requires(T strat)
{
    {strat.get_target_thread_id()} -> std::convertible_to<size_t>;
};





class CentralizedQueue
{
public:
    void push(WorkItem w, size_t thread_id);
    std::optional<WorkItem> pop(size_t thread_id);
private:
    std::queue<WorkItem> queue_;
    std::mutex queue_mutex_;
};

class RoundRobinStrategy
{
public:
    size_t get_target_thread_id();
private:
    size_t num_threads_;
    size_t next_thread_;
};



template 
<StorageStrategy StorageStrat, ScheduleStrategy ScheduleStrat>
class Pool
{
friend class PoolFactory;
public:
    void schedule(WorkItem w);
private:
    Pool();
    StorageStrat storage;
    ScheduleStrat scheduler;
};



template 
<StorageStrategy StorageStrat, ScheduleStrategy ScheduleStrat>
class PoolFactory
{
public:
    void reset();
    void setThreads(size_t thread_count);
    void setQueueSize(size_t queue_size);
    Pool<StorageStrat, ScheduleStrat get();
private:
    Pool product_;
};


} // namespace CoroExecutor
#endif // CORO_EXECUTOR_POOL_H