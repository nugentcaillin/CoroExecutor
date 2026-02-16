#ifndef CORO_EXECUTOR_POOL_H
#define CORO_EXECUTOR_POOL_H

#include <queue>
#include <mutex>
#include <CoroExecutor/ScheduleStrategy/ScheduleStrategy.hpp>

namespace CoroExecutor
{


struct WorkItem
{
    int priority;
    std::coroutine_handle<> h;
};


template <typename T>
concept StorageStrategy = requires(T strat, WorkItem w, size_t thread_id, size_t queue_size) 
{
    strat();
    strat(queue_size);
    strat.push(w, thread_id);
    {strat.pop(thread_id)} -> std::same_as<std::optional<WorkItem>>;
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




template 
<StorageStrategy StorageStrat, ScheduleStrategy ScheduleStrat>
class Pool
{
    template<StorageStrategy StorageStratB, ScheduleStrategy ScheduleStratB>
    friend class PoolFactory;
public:
    void schedule(WorkItem w);
private:
    Pool(size_t num_threads, size_t queue_size);
    StorageStrat storage;
    ScheduleStrat scheduler;
};



template 
<StorageStrategy StorageStrat, ScheduleStrategy ScheduleStrat>
class PoolFactory
{
public:
    PoolFactory();
    PoolFactory& set_threads(size_t thread_count);
    PoolFactory& setQueueSize(size_t queue_size);
    Pool<StorageStrat, ScheduleStrat>&& get();
private:
    size_t thread_count_;
    size_t queue_size_;
};


} // namespace CoroExecutor
#endif // CORO_EXECUTOR_POOL_H