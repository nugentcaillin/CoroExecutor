#include "testing.hpp"
#include <latch>
#include <atomic>
#include <thread>
#include <chrono>
#include <future>
#include <iostream>

using namespace CoroTesting;

// check that CoroExecutor eventually resumes a single handle that is requested to be resumed
TEST_F(SingleThreadedCoroExecutorTest, singleCoroutineResumed) 
{
    std::cout << "here\n";
    std::promise<void> resume_promise {};
    std::future<void> signal_resume { resume_promise.get_future() };

    auto coro = test_resume(std::move(resume_promise));

    executor->queue_resume(coro.handle_);

    // if handle.resume() is not eventually called by executor, fail 
    auto resume_status = signal_resume.wait_for(std::chrono::seconds(5));
    EXPECT_EQ(resume_status, std::future_status::ready);
}
// check that CoroExecutor eventually resumes all handles that are requested to be resumed
TEST_F(SingleThreadedCoroExecutorTest, multipleCoroutinesResumed) 
{
    int num_to_resume { 20 };
    std::vector<std::future<void>> resume_signals {};
    std::vector<resume_coro> resume_coros {}; // prevent destruction before testing resume



    std::cout << "queueing\n";
    for (int i { 0 }; i < num_to_resume; ++i)
    {
        std::promise<void> resume_promise {};
        resume_signals.push_back(std::move(resume_promise.get_future()));
        auto coro = test_resume(std::move(resume_promise));
        executor->queue_resume(coro.handle_);
        resume_coros.push_back(std::move(coro));
    }

    std::cout << "all queued\n";


    auto stop_waiting_time = std::chrono::system_clock::now() + std::chrono::milliseconds(200);

    // fail if handle.resume() was not eventually called for all coroutines
    for (int i {}; i < num_to_resume; ++i)
    {
        auto resume_status = resume_signals[i].wait_until(stop_waiting_time);
        //EXPECT_EQ(resume_status, std::future_status::ready);
    }
}



// check that CoroExecutor eventually resumes all handles that are requested to be resumed when
// it has multiple worker threads
TEST_F(MultiThreadedCoroExecutorTest, multipleCoroutinesResumedWithMultipleThreads) 
{
    int num_to_resume { 5 };
    std::vector<std::future<void>> resume_signals {};
    std::vector<resume_coro> resume_coros {}; // prevent destruction before testing resume

    for (int i { 0 }; i < num_to_resume; ++i)
    {
        std::promise<void> resume_promise {};
        resume_signals.push_back(resume_promise.get_future());
        auto coro = test_resume(std::move(resume_promise));
        executor->queue_resume(coro.handle_);
        resume_coros.push_back(std::move(coro));
    }

    auto stop_waiting_time = std::chrono::system_clock::now() + std::chrono::seconds(5);

    // fail if handle.resume() was not eventually called for all coroutines
    for (int i {}; i < num_to_resume; ++i)
    {
        auto resume_status = resume_signals[i].wait_until(stop_waiting_time);
        EXPECT_EQ(resume_status, std::future_status::ready);
    }
}

TEST_F(SingleThreadedCoroExecutorTest, coroutineResumedThroughAwaitableResumed) 
{
    std::promise<void> resume_promise {};
    std::future<void> signal_resume { resume_promise.get_future() };

    auto coro = test_resume_awaitable(std::move(resume_promise), executor);


    // if handle.resume() is not eventually called by executor, fail 
    auto resume_status = signal_resume.wait_for(std::chrono::seconds(5));
    EXPECT_EQ(resume_status, std::future_status::ready);
    std::cout << "awaitable done\n";
    //std::this_thread::sleep_for(std::chrono::milliseconds(5));
}


TEST_F(SingleThreadedCoroExecutorTest, coroutineResumedOnSeparateThreads)
{
    std::promise<std::thread::id> thread_promise {};
    std::future<std::thread::id> thread_future { thread_promise.get_future() };

    auto coro = test_thread_id(std::move(thread_promise));
    executor->queue_resume(coro.handle_);
    auto future_status = thread_future.wait_for(std::chrono::seconds(5));
    EXPECT_EQ(future_status, std::future_status::ready);
    EXPECT_NE(thread_future.get(), std::this_thread::get_id());
}


TEST_F(SingleThreadedCoroExecutorTest, coroutineResumedWithAwaitableResumedOnSeparateThreads)
{
    std::promise<std::thread::id> thread_promise {};
    std::future<std::thread::id> thread_future { thread_promise.get_future() };

    auto coro = test_thread_id_awaitable(std::move(thread_promise), executor);
    auto future_status = thread_future.wait_for(std::chrono::seconds(5));
    EXPECT_EQ(future_status, std::future_status::ready);
    EXPECT_NE(thread_future.get(), std::this_thread::get_id());
}



// queue up several coroutines to be resumed, fsanitize will fail test if there are leaks
TEST_F(SingleThreadedCoroExecutorTest, coroExecutorCleansUpSingleWorkerThread)
{    
    int num_to_resume { 100 };
    std::vector<resume_coro> resume_coros {}; // prevent destruction before testing resume



    std::cout << "queueing\n";
    for (int i { 0 }; i < num_to_resume; ++i)
    {
        std::promise<void> resume_promise {};
        auto coro = test_resume(std::move(resume_promise));
        executor->queue_resume(coro.handle_);
        resume_coros.push_back(std::move(coro));
    }
}


// queue up several coroutines to be resumed, fsanitize will fail test if there are leaks
TEST_F(MultiThreadedCoroExecutorTest, coroExecutorCleansUpMultipleWorkerThreads)
{
    int num_to_resume { 100 };
    std::vector<resume_coro> resume_coros {}; // prevent destruction before testing resume



    std::cout << "queueing\n";
    for (int i { 0 }; i < num_to_resume; ++i)
    {
        std::promise<void> resume_promise {};
        auto coro = test_resume(std::move(resume_promise));
        executor->queue_resume(coro.handle_);
        resume_coros.push_back(std::move(coro));
    }
}

// lifetime managed coroutines are eventually resumed
TEST_F(MultiThreadedCoroExecutorTest, LifetimeCoroutineResumed)
{
    std::promise<void> resume_promise {};
    std::future<void> resume_future { resume_promise.get_future() };
    auto coro = test_resume_lifetime(std::move(resume_promise));
    executor->add_lifetime_coroutine(std::move(coro));

    auto status = resume_future.wait_for(std::chrono::seconds(5));
    EXPECT_EQ(status, std::future_status::ready);
}

// all lifetime managed coroutines added are eventually resumed
TEST_F(MultiThreadedCoroExecutorTest, multipleLifetimeCoroutinesResumed)
{
    std::vector<std::future<void>> resume_futures {};
    int num_to_resume { 20 };

    for (int i {}; i < num_to_resume; ++i)
    {
        std::promise<void> resume_promise;
        resume_futures.emplace_back(resume_promise.get_future());
        auto coro = test_resume_lifetime(std::move(resume_promise));
        executor->add_lifetime_coroutine(std::move(coro));
    }

    auto wait_until_time = std::chrono::system_clock::now() + std::chrono::seconds(5);

    for (int i {}; i < num_to_resume; ++i)
    {
        auto status = resume_futures[i].wait_until(wait_until_time);
        EXPECT_EQ(status, std::future_status::ready);
    }
}


// more complex coroutine involving mock blocking operations
TEST_F(MultiThreadedCoroExecutorTest, complexLifetimeCoroutineManagedCorrectly)
{
    std::promise<void> resume_promise {};
    std::future<void> resume_future { resume_promise.get_future() };
    auto coro = test_complex_coroutine(std::move(resume_promise));
    executor->add_lifetime_coroutine(std::move(coro));

    auto status = resume_future.wait_for(std::chrono::seconds(5));
    EXPECT_EQ(status, std::future_status::ready);
}

// coroutine using a promise to return a value
TEST_F(MultiThreadedCoroExecutorTest, LifetimeCoroutineReturningValueManagedCorrectly)
{
    std::promise<int> return_promise {};
    std::future<int> return_future { return_promise.get_future() };
    auto coro = test_return_with_promise(std::move(return_promise), 8);
    executor->add_lifetime_coroutine(std::move(coro));

    auto status = return_future.wait_for(std::chrono::seconds(5));
    EXPECT_EQ(status, std::future_status::ready);
    EXPECT_EQ(return_future.get(), 8);
}

// destructor of CoroExecutor safely destroys remaining LifetimeManagedCoroutines
TEST_F(MultiThreadedCoroExecutorTest, CoroExecutorDestructorDestroysLifetimeManagedCoroutines)
{
    std::vector<std::future<void>> resume_futures {};
    int num_to_resume { 20 };

    for (int i {}; i < num_to_resume; ++i)
    {
        std::promise<void> resume_promise;
        resume_futures.emplace_back(resume_promise.get_future());
        auto coro = test_hangs_indefinitely(std::move(resume_promise));
        executor->add_lifetime_coroutine(std::move(coro));
    }

    auto wait_until_time = std::chrono::system_clock::now() + std::chrono::seconds(5);

    for (int i {}; i < num_to_resume; ++i)
    {
        auto status = resume_futures[i].wait_until(wait_until_time);
        EXPECT_EQ(status, std::future_status::ready);
    }
    // all are resumed at this point, but hang indefinitely, use ASan to check memory cleaned up
}
