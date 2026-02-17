#ifndef CORO_EXECUTOR_STORAGE_STRAT_H
#define CORO_EXECUTOR_STORAGE_STRAT_H

#include <concepts>
#include <cstddef>
#include <optional>
#include <CoroExecutor/Pool/WorkItem.hpp>

namespace CoroExecutor
{


template <typename T>
concept StorageStrategy = requires(T strat, WorkItem w, size_t thread_id, size_t queue_size) 
{
    T();
    T(queue_size);
    strat.push(w, thread_id);
    {strat.pop(thread_id)} -> std::same_as<std::optional<WorkItem>>;
};

} // namespace CoroExecutor
#endif // CORO_EXECUTOR_STORAGE_STRAT_H 