#ifndef CORO_EXECUTOR_ROUND_ROBIN_STRATEGY_H
#define CORO_EXECUTOR_ROUND_ROBIN_STRATEGY_H

#include <cstddef>
#include <atomic>
#include "ScheduleStrategy.hpp"

namespace CoroExecutor
{


class RoundRobinStrategy
{
public:
    RoundRobinStrategy(size_t num_threads);
    size_t get_target_thread_id();
private:

    std::atomic<size_t> next_thread_;
    size_t num_threads_;
};


static_assert(ScheduleStrategy<RoundRobinStrategy>);

} // namespace CoroExecutor
#endif // CORO_EXECUTOR_ROUND_ROBIN_STRATEGY_H