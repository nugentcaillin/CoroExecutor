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





// execution should resume when value recieved with co_await 
TEST(BasicTest, ExecutionResumesAfterCoAwaitReturnsValue)
{
    std::promise<int> sentinel;
    std::future<int> recieved = sentinel.get_future();
    auto return_coro = return_value(3);
    CoroExecutor::Task<int> coro = recieve_val_with_co_await(3, std::move(sentinel), return_coro);
    coro.handle_.resume();
    return_coro.handle_.resume();
    auto status = recieved.wait_for(std::chrono::seconds(1));
    ASSERT_EQ(status, std::future_status::ready);
    ASSERT_EQ(recieved.get(), 3);
}

// execution should not resume if value not received with co_await
TEST(BasicTest, ExecutionDoesntResumeIfCoAwaitHangs)
{
    std::promise<int> sentinel;
    std::future<int> recieved = sentinel.get_future();
    auto return_coro = return_value(3);
    CoroExecutor::Task<int> coro = recieve_val_with_co_await(3, std::move(sentinel), return_coro);
    coro.handle_.resume();
    auto status = recieved.wait_for(std::chrono::seconds(1));
    ASSERT_EQ(status, std::future_status::timeout);
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
    MultiCoAwaitTestSingleThreaded()
    : MultiCoAwaitTest(5, 3)
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
    MultiCoAwaitTestMultiThreaded()
    : MultiCoAwaitTest(100, 3)
    , threads_ {}
    {}

    void SetUp() override
    {
        MultiCoAwaitTest::SetUp();
        for (auto coro : receiving_coroutines_)
        {
            threads_.push_back(std::thread(concurrent_resume, coro));
        }
    }

    void TearDown() override 
    {
        for (auto& thread : threads_) thread.join();
    }
};


TEST_F(MultiCoAwaitTestSingleThreaded, AllAwaitersSuspend)
{
    ASSERT_EQ(count_ready(), 0); 
}


TEST_F(MultiCoAwaitTestSingleThreaded, AllAwaitersGetCorrectValue)
{
    coro_awaited_many_times_.handle_.resume();
    ASSERT_EQ(count_received_correct_value(), num_to_await_);
}


TEST_F(MultiCoAwaitTestMultiThreaded, AllAwaitersSuspend)
{
    ASSERT_EQ(count_ready(), 0); 
}


TEST_F(MultiCoAwaitTestMultiThreaded, AllAwaitersGetCorrectValue)
{
    coro_awaited_many_times_.handle_.resume();
    ASSERT_EQ(count_received_correct_value(), num_to_await_);
}


