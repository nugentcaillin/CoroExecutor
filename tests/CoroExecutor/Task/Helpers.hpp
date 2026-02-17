#ifndef CORO_EXECUTOR_TEST_HELPER_H
#define CORO_EXECUTOR_TEST_HELPER_H

#include <CoroExecutor/CoroExecutor.hpp>
#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include <future>


struct DestructorSentinel;
void concurrent_copy(CoroExecutor::Task<int> coro);
void concurrent_resume(CoroExecutor::Task<int> coro);
CoroExecutor::Task<int> test_destruction(std::vector<bool>& flags, int flag_index);
CoroExecutor::Task<int> return_value(int val);
CoroExecutor::Task<int> recieve_val_with_co_await(int expected_val, std::promise<int> sentinel, CoroExecutor::Task<int> receive_fn);



struct DestructorSentinel {
    std::vector<bool>& flags_;
    int flag_index_;
    DestructorSentinel(std::vector<bool>& flag, int index)
    : flags_(flag)
    , flag_index_ { index }
    {};

    ~DestructorSentinel()
    {
        flags_.at(flag_index_) = true;
    }
};


void concurrent_copy(CoroExecutor::Task<int> coro)
{
    auto now = std::chrono::steady_clock::now();
    // naive synchronization to give ourselves a greater chance of triggering a race condition
    using ms100 = std::chrono::duration<double, std::ratio<1, 10>>;
    auto resume_point = std::chrono::ceil<ms100>(now);
    std::this_thread::sleep_until(resume_point);
    CoroExecutor::Task<int> copy = coro;
}

CoroExecutor::Task<int> test_destruction(std::vector<bool>& flags, int flag_index)
{
    DestructorSentinel sentinel(flags, flag_index);
    co_await std::suspend_always {}; // to keep sentinel alive until handle.destroy() called
    co_return 1;
}


CoroExecutor::Task<int> return_value(int val)
{
    co_return val;
}


CoroExecutor::Task<int> recieve_val_with_co_await(int expected_val, std::promise<int> sentinel, CoroExecutor::Task<int> receive_fn)
{
    int val = co_await receive_fn;
    if (val == expected_val) sentinel.set_value(val);
    else sentinel.set_value(false);
    co_return 0;
}


void concurrent_resume(CoroExecutor::Task<int> coro)
{
    auto now = std::chrono::steady_clock::now();
    // naive synchronization to give ourselves a greater chance of triggering a race condition
    using ms100 = std::chrono::duration<double, std::ratio<1, 10>>;
    auto resume_point = std::chrono::ceil<ms100>(now);
    std::this_thread::sleep_until(resume_point);
    coro.handle_.resume();
}



#endif // CORO_EXECUTOR_TEST_HELPER_H