#ifndef CORO_EXECUTOR_TEST_TASK_FIXTURE_H
#define CORO_EXECUTOR_TEST_TASK_FIXTURE_H

#include <CoroExecutor/Umbrella.hpp>
#include "Helpers.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <ranges>
#include <thread>
#include <future>



class TaskDestructionTest : public testing::Test
{
protected:
    int num_tasks_not_empty_;
    std::vector<bool> frames_destroyed_;
    std::vector<CoroExecutor::Task<int>> tasks_;
    TaskDestructionTest(int num_tasks_not_empty)
    : num_tasks_not_empty_ { num_tasks_not_empty }
    , frames_destroyed_(num_tasks_not_empty)
    , tasks_ {} 
    {}

    void SetUp() override
    {
        for ( int i : std::ranges::views::iota(0, num_tasks_not_empty_)) 
        {
            tasks_.push_back(test_destruction(frames_destroyed_, i));
            tasks_.back().handle_.resume();
        }
    }
};

class MultiCoAwaitTest : public testing::Test {
protected:
    int num_to_await_;
    int expected_val_;
    std::vector<std::future<int>> received_value_;
    std::vector<CoroExecutor::Task<int>> receiving_coroutines_;
    CoroExecutor::Task<int> coro_awaited_many_times_;

    MultiCoAwaitTest(int num_to_await, int expected_val)
    : num_to_await_ { num_to_await }
    , expected_val_ { expected_val }
    , received_value_ {}
    , receiving_coroutines_ {}
    , coro_awaited_many_times_(return_value(expected_val))
    {}

    MultiCoAwaitTest() : MultiCoAwaitTest(5, 3) {}

    void SetUp() override
    {
        for ([[maybe_unused]] int _: std::ranges::views::iota(0, num_to_await_))
        {
            std::promise<int> sentinel {};
            received_value_.push_back(sentinel.get_future()); 
            receiving_coroutines_.push_back(recieve_val_with_co_await(expected_val_, std::move(sentinel), coro_awaited_many_times_));
        }
    }

    int count_received_correct_value()
    {
        auto wait_until_point = std::chrono::steady_clock::now() + std::chrono::seconds(1);
        int count {};
        for (std::future<int>& received : received_value_)
        {
            auto status = received.wait_until(wait_until_point); 
            if (status == std::future_status::ready && received.get() == expected_val_) ++count;
        }
        return count;
    }

    int count_ready()
    {
        auto wait_until_point = std::chrono::steady_clock::now();
        int count {};
        for (std::future<int>& received : received_value_)
        {
            auto status = received.wait_until(wait_until_point); 
            if (status == std::future_status::ready) ++count;

        }
        return count;
    }
};



class MultiCoAwaitTestSingleThreaded: public MultiCoAwaitTest 
{
protected:
    MultiCoAwaitTestSingleThreaded(int num_to_await, int expected_val)
    : MultiCoAwaitTest(num_to_await, expected_val)
    {}

    MultiCoAwaitTestSingleThreaded()
    : MultiCoAwaitTestSingleThreaded(5, 3)
    {}

    void SetUp() override
    {
        MultiCoAwaitTest::SetUp();
        for (auto coro : receiving_coroutines_) coro.handle_.resume();
    }

};



class MultiCoAwaitTestMultiThreaded : public MultiCoAwaitTest 
{
protected:
    std::vector<std::thread> threads_;
    void(*concurrent_func_)(CoroExecutor::Task<int>);
    MultiCoAwaitTestMultiThreaded(int num_threads, int expected_val, void(*concurrent_func)(CoroExecutor::Task<int>))
    : MultiCoAwaitTest(num_threads, expected_val)
    , threads_ {}
    , concurrent_func_ { concurrent_func }
    {}

    MultiCoAwaitTestMultiThreaded()
    : MultiCoAwaitTestMultiThreaded(100, 3, concurrent_resume)
    {}

    void SetUp() override
    {
        MultiCoAwaitTest::SetUp();
        for (auto coro : receiving_coroutines_)
        {
            threads_.push_back(std::thread(concurrent_func_, coro));
        }
    }

    void TearDown() override 
    {
        for (auto& thread : threads_) thread.join();
    }
};


class SingleCoAwaitTest : public MultiCoAwaitTestSingleThreaded
{
protected:
    SingleCoAwaitTest()
    : MultiCoAwaitTestSingleThreaded(1, 3)
    {}
};



class SmallTaskDestructionTest : public TaskDestructionTest
{
protected:
    SmallTaskDestructionTest()
    : TaskDestructionTest(2)
    {}
};


class MultiThreadedCopyStressTest : public TaskDestructionTest
{
protected:
    std::vector<std::thread> threads_;
    int num_threads_;
    MultiThreadedCopyStressTest(int num_threads)
    : TaskDestructionTest(1)
    , threads_ {}
    , num_threads_ { num_threads }
    {}

    MultiThreadedCopyStressTest()
    : MultiThreadedCopyStressTest(1000)
    {}


    void SetUp() override
    {
        TaskDestructionTest::SetUp();

        for ([[maybe_unused]] int _ : std::ranges::views::iota(0, num_threads_))
        {
            threads_.push_back(std::thread(concurrent_copy, tasks_.front()));
        } 
    }

    void joinAll()
    {
        for (auto& thread : threads_)
        {
            if (thread.joinable()) thread.join();
        }
    }

    void TearDown() override 
    {
        joinAll();
    }

};

#endif // CORO_EXECUTOR_TEST_TASK_FIXTURE_H