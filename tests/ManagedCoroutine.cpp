#include <CoroExecutor/CoroExecutor.hpp>
#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include <ranges>
#include <thread>
#include <future>


struct DestructorSentinel {
    bool& flag_;
    DestructorSentinel(bool& flag)
    : flag_(flag)
    {};

    ~DestructorSentinel()
    {
        flag_ = true;
    }
};

CoroExecutor::Task<int> test_destruction(bool& flag)
{
    DestructorSentinel sentinel(flag);
    co_await std::suspend_always {}; // to keep sentinel alive until handle.destroy() called
    co_return 1;
}

// frame should only destroyed once with copy assignent
TEST(BasicTest, TaskCoroutineFrameDestroyedOnceWithCopyAssignment)
{
    bool coroutine_frame_destroyed {};
    {
        auto first = test_destruction(coroutine_frame_destroyed);
        first.handle_.resume(); // so sentinel is created
        {
            auto second = first;
            EXPECT_EQ(coroutine_frame_destroyed, false);
        }
        EXPECT_EQ(coroutine_frame_destroyed, false);
    }
    EXPECT_EQ(coroutine_frame_destroyed, true);
}

// self copy assignment should also result in frame being destroyed only once and at appropriate point
TEST(BasicTest, TaskCoroutineFrameDestroyedOnceWithSelfCopyAssignment)
{
    bool coroutine_frame_destroyed {};
    {
        auto first = test_destruction(coroutine_frame_destroyed);
        first.handle_.resume(); // so sentinel is created
        {
            first = first;
            EXPECT_EQ(coroutine_frame_destroyed, false);
        }
        EXPECT_EQ(coroutine_frame_destroyed, false);
    }
    EXPECT_EQ(coroutine_frame_destroyed, true);
}

// on copy assignment, if task copied into is last reference, should have frame destroyed
TEST(BasicTest, TaskOverwritesOnCopyAssignment)
{
    bool first_coroutine_frame_destroyed {};
    bool second_coroutine_frame_destroyed {};
    {
        auto first = test_destruction(first_coroutine_frame_destroyed);
        first.handle_.resume(); // so sentinel is created
        {
            auto second = test_destruction(second_coroutine_frame_destroyed);
            second.handle_.resume();
            EXPECT_EQ(first_coroutine_frame_destroyed, false);
            EXPECT_EQ(second_coroutine_frame_destroyed, false);
            first = second;
            EXPECT_EQ(first_coroutine_frame_destroyed, true);
        }
        EXPECT_EQ(second_coroutine_frame_destroyed, false);
    }
    EXPECT_EQ(second_coroutine_frame_destroyed, true);
}

// copy construction should result in only one destruction of frame
TEST(BasicTest, TaskCoroutineFrameDestroyedOnceWithCopyConstructor)
{
    bool coroutine_frame_destroyed {};
    {
        auto first = test_destruction(coroutine_frame_destroyed);
        first.handle_.resume(); // so sentinel is created
        {
            CoroExecutor::Task<int> second(first);
            EXPECT_EQ(coroutine_frame_destroyed, false);
        }
        EXPECT_EQ(coroutine_frame_destroyed, false);
    }
    EXPECT_EQ(coroutine_frame_destroyed, true);
}

// coroutine frame should be destroyed exactly once after move assignment
TEST(BasicTest, TaskCoroutineFrameDestroyedOnceWithMoveAssignment)
{
    bool coroutine_frame_destroyed {};
    {
        auto first = test_destruction(coroutine_frame_destroyed);
        first.handle_.resume(); // so sentinel is created
        {
            auto second = std::move(first);
            EXPECT_EQ(coroutine_frame_destroyed, false);
        }
        EXPECT_EQ(coroutine_frame_destroyed, true);
    }
}

// when moving into non empty task that is last reference, that task should have its frame destroyed
TEST(BasicTest, TaskCoroutineFrameDestroyedOnceWithMoveAssignmentIntoNonEmpty)
{
    bool first_coroutine_frame_destroyed {};
    bool second_coroutine_frame_destroyed {};
    {
        auto first = test_destruction(first_coroutine_frame_destroyed);
        first.handle_.resume(); // so sentinel is created
        {
            auto second = test_destruction(second_coroutine_frame_destroyed);
            second.handle_.resume(); // so sentinel is created
            first = std::move(second);
            EXPECT_EQ(first_coroutine_frame_destroyed, true);
            EXPECT_EQ(second_coroutine_frame_destroyed, false);
        }
        EXPECT_EQ(second_coroutine_frame_destroyed, false);
    }
    EXPECT_EQ(second_coroutine_frame_destroyed, true);
}

// coroutine handle and frame destruction should be handled correctly on self move assignent
TEST(BasicTest, TaskCoroutineFrameDestroyedOnceWithMoveAssignmentSelfAssignment)
{
    bool coroutine_frame_destroyed {};
    {
        auto first = test_destruction(coroutine_frame_destroyed);
        first.handle_.resume(); // so sentinel is created
        {
            first = std::move(first);
            EXPECT_EQ(coroutine_frame_destroyed, false);
        }
        EXPECT_EQ(coroutine_frame_destroyed, false);
    }
    EXPECT_EQ(coroutine_frame_destroyed, true);
}

// coroutine frame should be destroyed exactly once after move
TEST(BasicTest, TaskCoroutineFrameDestroyedOnceWithMoveConstructor)
{
    bool coroutine_frame_destroyed {};
    {
        auto first = test_destruction(coroutine_frame_destroyed);
        first.handle_.resume(); // so sentinel is created
        {
            auto second(std::move(first));
            EXPECT_EQ(coroutine_frame_destroyed, false);
        }
        EXPECT_EQ(coroutine_frame_destroyed, true);
    }
}

// coroutine frames should swap handles appropriately
TEST(BasicTest, TaskCoroutineFrameSwapsCorrectly)
{
    bool first_coroutine_frame_destroyed {};
    bool second_coroutine_frame_destroyed {};
    {
        auto first = test_destruction(first_coroutine_frame_destroyed);
        first.handle_.resume(); // so sentinel is created
        {
            auto second = test_destruction(second_coroutine_frame_destroyed);
            second.handle_.resume(); // so sentinel is created
            std::swap(first, second);
            EXPECT_EQ(first_coroutine_frame_destroyed, false);
            EXPECT_EQ(second_coroutine_frame_destroyed, false);
        }
        EXPECT_EQ(first_coroutine_frame_destroyed, true);
        EXPECT_EQ(second_coroutine_frame_destroyed, false);
    }
    EXPECT_EQ(second_coroutine_frame_destroyed, true);
}


void concurrent_copy(CoroExecutor::Task<int> coro)
{
    auto now = std::chrono::steady_clock::now();
    // naive synchronization to give ourselves a greater chance of triggering a race condition
    auto resume_point = std::chrono::ceil<std::chrono::seconds>(now);
    std::this_thread::sleep_until(resume_point);
    CoroExecutor::Task<int> copy = coro;
}

// copies should be able to be made and destroyed on separate threads concurrently with coroutine frame being destroyed exactly once
TEST(BasicTest, TasksCanBeDestroyedSimultaneouslyOnMultipleThreads)
{
    const int NUM_THREADS { 1000 };
    bool coroutine_frame_destroyed {};
    // check coroutine frame not destroyed after many concurrent attempts at destruction if one reference remains
    {
        std::vector<std::thread> threads {};
        threads.reserve(NUM_THREADS);
        
        auto coro = test_destruction(coroutine_frame_destroyed);
        coro.handle_.resume(); // to create sentinel

        for ([[maybe_unused]]int _ : std::ranges::views::iota(0, NUM_THREADS))
        {
            std::thread t(concurrent_copy, coro);
            threads.push_back(std::move(t));
        }

        // join
        for ([[maybe_unused]]int pos : std::ranges::views::iota(0, NUM_THREADS))
        {
            threads.at(pos).join();
        }
        // should have one task left, so shouldn't have been destroyed
        ASSERT_EQ(coroutine_frame_destroyed, false);
    }
    ASSERT_EQ(coroutine_frame_destroyed, true);

    coroutine_frame_destroyed = false;
    // check frame is destroyed after many concurrent attempts at destruction happen with count of task references reaching 0
    {
        std::vector<std::thread> threads {};
        threads.reserve(NUM_THREADS);
        { 
        auto coro = test_destruction(coroutine_frame_destroyed);
        coro.handle_.resume(); // to create sentinel

            for ([[maybe_unused]]int _ : std::ranges::views::iota(0, NUM_THREADS))
            {
                std::thread t(concurrent_copy, coro);
                threads.push_back(std::move(t));
            }
        } // original Task out of scope, when last copy in threads falls out of scope, frame should be destroyed

        // join
        for ([[maybe_unused]]int pos : std::ranges::views::iota(0, NUM_THREADS))
        {
            threads.at(pos).join();
        }
        ASSERT_EQ(coroutine_frame_destroyed, true);
    }

    // 
}

CoroExecutor::Task<int> return_value(int val)
{
    co_return val;
}


CoroExecutor::Task<int> fail_to_return_val(int val)
{
    co_await std::suspend_always {}; // get stuck
    co_return val;
}


CoroExecutor::Task<int> recieve_val_with_co_await(int expected_val, std::promise<bool>& sentinel, CoroExecutor::Task<int> receive_fn)
{
    receive_fn.handle_.resume();
    int val = co_await receive_fn;
    if (val == expected_val) sentinel.set_value(true);
    else sentinel.set_value(false);
    co_return 0;
}





// execution should resume when value recieved with co_await 
TEST(BasicTest, ExecutionResumesAfterCoAwaitReturnsValue)
{
    std::promise<bool> sentinel;
    std::future<bool> recieved = sentinel.get_future();
    CoroExecutor::Task<int> coro = recieve_val_with_co_await(3, sentinel, return_value(3));
    coro.handle_.resume();
    auto status = recieved.wait_for(std::chrono::seconds(1));
    ASSERT_EQ(status, std::future_status::ready);
    ASSERT_EQ(recieved.get(), true);
}

// execution should not resume if value not received with co_await
TEST(BasicTest, ExecutionDoesntResumeIfCoAwaitHangs)
{
    std::promise<bool> sentinel;
    std::future<bool> recieved = sentinel.get_future();
    CoroExecutor::Task<int> coro = recieve_val_with_co_await(3, sentinel, fail_to_return_val(3));
    coro.handle_.resume();
    auto status = recieved.wait_for(std::chrono::seconds(1));
    ASSERT_EQ(status, std::future_status::timeout);
}
