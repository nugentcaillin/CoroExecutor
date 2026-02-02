#include <CoroExecutor/CoroExecutor.hpp>
#include <gtest/gtest.h>
#include <future>
#include <chrono>


CoroExecutor::MangagedCoroutine signalOnFirstResume(std::promise<bool>& signal)
{
    signal.set_value(true);
    co_return;
}

TEST(BasicTest, ManagedCoroutineExecutesLazily)
{
    std::promise<bool> signalFirstResume {};
    std::future<bool> signal = signalFirstResume.get_future();

    auto coro = signalOnFirstResume(signalFirstResume);
    auto hasResumed = signal.wait_for(std::chrono::milliseconds(1)); 

    EXPECT_EQ(hasResumed, std::future_status::timeout);
    coro.handle_.resume();

    hasResumed = signal.wait_for(std::chrono::milliseconds(10)); 
    EXPECT_EQ(hasResumed, std::future_status::ready);
    coro.handle_.destroy();
}

