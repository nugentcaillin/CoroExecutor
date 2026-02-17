#include <gtest/gtest.h>
#include "Fixtures.hpp"

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