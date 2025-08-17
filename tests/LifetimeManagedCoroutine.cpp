#include "testing.hpp"
#include <iostream>
#include <latch>
#include <atomic>

TEST_F(CoroExecutorTest, waitsForResume)
{
    std::latch resume_latch { 1 };
    std::atomic<int> resume_counter { 0 };
    std::thread::id captured_id {};
    auto coro = test_coro(resume_latch, resume_counter, captured_id);

    EXPECT_EQ(resume_counter, 0);

    destroy_helper(coro);
}


TEST_F(CoroExecutorTest, executeAfterResume)
{
    std::latch resume_latch { 1 };
    std::atomic<int> resume_counter { 0 };
    std::thread::id captured_id {};
    auto coro = test_coro(resume_latch, resume_counter, captured_id);

    resume_helper(coro);
    resume_latch.wait();

    EXPECT_EQ(resume_counter, 1);

}
