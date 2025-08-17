#include "testing.hpp"
#include <latch>
#include <atomic>


TEST_F(CoroExecutorTest, singleCoroutineResumed) 
{
    std::shared_ptr<CoroExecutor::CoroExecutor> exec = std::make_shared<CoroExecutor::CoroExecutor>(1);

    std::latch resume_latch { 1 };
    std::atomic<int> resume_counter { 0 };
    std::thread::id captured_id {};
    auto coro = test_coro(resume_latch, resume_counter, captured_id);

    exec->queue_resume(handle_helper(coro));

    resume_latch.wait();

    EXPECT_EQ(resume_counter, 1);
}


TEST_F(CoroExecutorTest, multipleCoroutinesResumed) 
{
    std::shared_ptr<CoroExecutor::CoroExecutor> exec = std::make_shared<CoroExecutor::CoroExecutor>(1);

    std::latch resume_latch { 5 };
    std::atomic<int> resume_counter { 0 };
    std::thread::id captured_id {};

    for (int i { 0 }; i < 5; ++i)
    {
        auto coro = test_coro(resume_latch, resume_counter, captured_id);
        exec->queue_resume(handle_helper(coro));
    }
    resume_latch.wait();

    EXPECT_EQ(resume_counter, 5);

}


TEST_F(CoroExecutorTest, multipleCoroutinesResumedWithMultipleThreads) 
{
    std::shared_ptr<CoroExecutor::CoroExecutor> exec = std::make_shared<CoroExecutor::CoroExecutor>(4);

    std::latch resume_latch { 20 };
    std::atomic<int> resume_counter { 0 };
    std::thread::id captured_id {};

    for (int i { 0 }; i < 20; ++i)
    {
        auto coro = test_coro(resume_latch, resume_counter, captured_id);
        exec->queue_resume(handle_helper(coro));
    }
    resume_latch.wait();

    EXPECT_EQ(resume_counter, 20);
}



TEST_F(CoroExecutorTest, coroutineResumedThroughAwaitableResumed) 
{
    std::shared_ptr<CoroExecutor::CoroExecutor> exec = std::make_shared<CoroExecutor::CoroExecutor>(1);

    std::latch resume_latch { 1 };
    std::atomic<int> resume_counter { 0 };
    std::thread::id captured_id {};

    auto coro = queue_resumption_with_awaitable(resume_latch, resume_counter, captured_id, exec);
    resume_latch.wait();

    EXPECT_EQ(resume_counter, 1);
}


TEST_F(CoroExecutorTest, coroutineResumedOnSeparateThreads)
{
    std::shared_ptr<CoroExecutor::CoroExecutor> exec = std::make_shared<CoroExecutor::CoroExecutor>(1);

    std::latch resume_latch { 1 };
    std::atomic<int> resume_counter { 0 };
    std::thread::id captured_id {};
    auto coro = test_coro(resume_latch, resume_counter, captured_id);

    exec->queue_resume(handle_helper(coro));

    resume_latch.wait();

    EXPECT_FALSE(std::this_thread::get_id() == captured_id);
}


TEST_F(CoroExecutorTest, coroutineResumedWithAwaitableResumedOnSeparateThreads)
{
    std::shared_ptr<CoroExecutor::CoroExecutor> exec = std::make_shared<CoroExecutor::CoroExecutor>(1);

    std::latch resume_latch { 1 };
    std::atomic<int> resume_counter { 0 };
    std::thread::id captured_id {};

    auto coro = queue_resumption_with_awaitable(resume_latch, resume_counter, captured_id, exec);
    resume_latch.wait();

    EXPECT_FALSE(std::this_thread::get_id() == captured_id);
}

TEST_F(CoroExecutorTest, coroExecutorCleansUpSingleWorkerThread)
{
    std::shared_ptr<CoroExecutor::CoroExecutor> exec = std::make_shared<CoroExecutor::CoroExecutor>(1);

    std::latch resume_latch { 5 };
    std::atomic<int> destruction_counter { 0 };

    for (int i { 0 }; i < 5; ++i)
    {
        auto coro = count_destruction_with_awaitable(resume_latch, destruction_counter, exec);
    }
    resume_latch.wait();

    EXPECT_EQ(destruction_counter, 5);
}


TEST_F(CoroExecutorTest, coroExecutorCleansUpMultipleWorkerThreads)
{
    std::shared_ptr<CoroExecutor::CoroExecutor> exec = std::make_shared<CoroExecutor::CoroExecutor>(4);

    std::latch resume_latch { 20 };
    std::atomic<int> destruction_counter { 0 };

    for (int i { 0 }; i < 20; ++i)
    {
        auto coro = count_destruction_with_awaitable(resume_latch, destruction_counter, exec);
    }
    resume_latch.wait();

    EXPECT_EQ(destruction_counter, 20);
}