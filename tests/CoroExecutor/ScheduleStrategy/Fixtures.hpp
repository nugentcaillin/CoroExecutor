#ifndef CORO_EXECUTOR_TEST_SCHEDULE_STRATEGY_H
#define CORO_EXECUTOR_TEST_SCHEDULE_STRATEGY_H

#include <gtest/gtest.h>

#include <ranges>

#include <CoroExecutor/ScheduleStrategy/Umbrella.hpp>

#include <SharedHelpers.hpp>

class RoundRobinTest : public testing::Test, public AttemptRaceCondition 
{
protected:
    size_t num_dummy_threads_;
    size_t advances_per_thread_;
    CoroExecutor::RoundRobinStrategy round_robin_;
    std::vector<size_t> frequencies_;

    void advance(size_t calls)
    {
        for ([[maybe_unused]]int _ : std::ranges::views::iota(0u, calls)) round_robin_.get_target_thread_id();
    }

    RoundRobinTest(size_t num_dummy_threads, size_t advances_per_thread)
    : num_dummy_threads_ { num_dummy_threads }
    , advances_per_thread_ { advances_per_thread }
    , round_robin_ { num_dummy_threads }
    , frequencies
    {}
    RoundRobinTest()
    : RoundRobinTest(5, 1000)
    {}

    void attempt_one() override 
    {
        advance(advances_per_thread_);
    }
};

#endif // CORO_EXECUTOR_TEST_SCHEDULE_STRATEGY_H