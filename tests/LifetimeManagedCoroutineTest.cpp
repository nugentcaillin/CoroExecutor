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
    
}

TEST_F(CoroExecutorTest, executesAfterResume)
{
    std::latch resume_latch { 1 };
    std::atomic<int> resume_counter { 0 };
    std::thread::id captured_id {};
    auto coro = test_coro(resume_latch, resume_counter, captured_id);

    auto handle = handle_helper(coro);

    handle.resume();
    resume_latch.wait();

    EXPECT_EQ(resume_counter, 1);

    handle.destroy();
}
