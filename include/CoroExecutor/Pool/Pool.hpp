#ifndef CORO_EXECUTOR_POOL_H
#define CORO_EXECUTOR_POOL_H

#include <queue>
#include <mutex>
#include <CoroExecutor/ScheduleStrategy/ScheduleStrategy.hpp>
#include <CoroExecutor/StorageStrategy/StorageStrategy.hpp>

namespace CoroExecutor
{










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