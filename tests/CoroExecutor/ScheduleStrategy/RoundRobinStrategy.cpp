#include <gtest/gtest.h>
#include "Fixtures.hpp"
#include <stdexcept>

#include <CoroExecutor/ScheduleStrategy/RoundRobinStrategy.hpp>

TEST(RoundRobinBasicTest, zeroThreadsGivesError)
{
    ASSERT_THROW(CoroExecutor::RoundRobinStrategy(0), std::invalid_argument);
}

TEST_F(RoundRobinTest, firstThreadIDIsZero)
{
    ASSERT_EQ(round_robin_.get_target_thread_id(), 0u);
}

TEST_F(RoundRobinTest, threadIDIncrements)
{
    advance(1);
    ASSERT_EQ(round_robin_.get_target_thread_id(), 1u);
}

TEST_F(RoundRobinTest, threadIDLoopsRoundAtThreadCount)
{
    advance(num_dummy_threads_);
    ASSERT_EQ(round_robin_.get_target_thread_id(), 0u);
}

TEST_F(RoundRobinTest, ManyIDRequestsConcurrentlyGiveCorrectID)
{
    const size_t num_threads { (size_t)std::thread::hardware_concurrency() };
    attempt_many(num_threads);
    join_all();
    ASSERT_EQ(round_robin_.get_target_thread_id(), ((0u + num_threads) * advances_per_thread_) % num_dummy_threads_);
}