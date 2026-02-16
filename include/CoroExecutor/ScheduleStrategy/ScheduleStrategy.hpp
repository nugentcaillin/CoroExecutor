#ifndef CORO_EXECUTOR_SCHEDULE_STRATEGY_H
#define CORO_EXECUTOR_SCHEDULE_STRATEGY_H

#include <concepts>


namespace CoroExecutor
{


template<typename T>
concept ScheduleStrategy = requires(T strat)
{
    {strat.get_target_thread_id()} -> std::same_as<size_t>;
};


} // namespace CoroExecutor
#endif // CORO_EXECUTOR_SCHEDULE_STRATEGY_H