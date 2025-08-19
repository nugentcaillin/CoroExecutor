#include "testing.hpp"
#include <iostream>
#include <latch>
#include <atomic>


TEST_F(CoroExecutorTest, waitsForResume)
{
    std::cout << "begin\n";
    std::latch resume_latch { 1 };
    std::atomic<int> resume_counter { 0 };
    std::thread::id captured_id {};
    std::cout << "locals done\n";
    auto coro = test_coro(resume_latch, resume_counter, captured_id);
    std::cout << "coro created\n";

    EXPECT_EQ(resume_counter, 0);
    
}

TEST_F(CoroExecutorTest, executesAfterResume)
{
    std::cout << "begin\n";
    std::latch resume_latch { 1 };
    std::atomic<int> resume_counter { 0 };
    std::thread::id captured_id {};
    std::cout << "locals done\n";
    auto coro = test_coro(resume_latch, resume_counter, captured_id);
    std::cout << "coro created\n";

    auto handle = handle_helper(coro);
    std::cout << "handle obtained\n";

    handle.resume();
    std::cout << "handle finished resuming\n";
    resume_latch.wait();
    std::cout << "latch done\n";

    EXPECT_EQ(resume_counter, 1);

    handle.destroy();
}
