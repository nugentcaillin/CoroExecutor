#ifndef CORO_EXECUTOR_CENTRALIZED_QUEUE_STRAT_H
#define CORO_EXECUTOR_CENTRALIZED_QUEUE_STRAT_H

#include <cstddef>
#include <queue>
#include <optional>
#include <mutex>
#include <CoroExecutor/StorageStrategy/StorageStrategy.hpp>

namespace CoroExecutor
{
class CentralizedQueueStrategy
{
public:
    CentralizedQueueStrategy();
    CentralizedQueueStrategy(size_t num_threads);
    void push(WorkItem w, size_t thread_id);
    std::optional<WorkItem> pop(size_t thread_id);
private:
    std::queue<WorkItem> queue_;
    std::mutex queue_mutex_;
};

static_assert(StorageStrategy<CentralizedQueueStrategy>);

} // namespace CoroExecutor
#endif // CORO_EXECUTOR_CENTRALIZED_QUEUE_STRAT_H