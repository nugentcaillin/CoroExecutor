#ifndef CORO_EXECUTOR_ROUND_ROBIN_STRATEGY_H
#define CORO_EXECUTOR_ROUND_ROBIN_STRATEGY_H

#include <cstddef>
#include "ScheduleStrategy.hpp"

namespace CoroExecutor
{


class RoundRobinStrategy
{
public:
    size_t get_target_thread_id();
private:
    size_t num_threads_;
    size_t next_thread_;
};


static_assert(ScheduleStrategy<RoundRobinStrategy>);

} // namespace CoroExecutor
#endif // CORO_EXECUTOR_ROUND_ROBIN_STRATEGY_H