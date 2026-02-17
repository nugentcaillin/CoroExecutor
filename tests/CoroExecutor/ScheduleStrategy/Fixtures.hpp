#ifndef CORO_EXECUTOR_TEST_SCHEDULE_STRATEGY_H
#define CORO_EXECUTOR_TEST_SCHEDULE_STRATEGY_H

#include <gtest/gtest.h>

#include <ranges>

#include <CoroExecutor/ScheduleStrategy/Umbrella.hpp>

class RoundRobinTest : public testing::Test 
{
protected:
    size_t num_dummy_threads_;
    CoroExecutor::RoundRobinStrategy round_robin_;
    void advance(size_t calls)
    {
        for ([[maybe_unused]]int _ : std::ranges::views::iota(0u, calls)) round_robin_.get_target_thread_id();
    }

    RoundRobinTest(size_t num_dummy_threads)
    : num_dummy_threads_ { num_dummy_threads }
    , round_robin_ { num_dummy_threads }
    {}
    RoundRobinTest()
    : RoundRobinTest(5)
    {}
};

#endif // CORO_EXECUTOR_TEST_SCHEDULE_STRATEGY_H