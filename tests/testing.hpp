#ifndef TESTING_H
#define TESTING_H

#include <gtest/gtest.h>
#include <CoroExecutor/CoroExecutor.hpp>
#include <latch>
#include <thread>
#include <atomic>
#include <coroutine>
#include <chrono>
#include <future>
#include <memory>
#include <iostream>


namespace CoroTesting
{
    // initial suspend always, automatically destroy at end
    struct resume_coro
    {
        struct promise_type
        {
            using handle = std::coroutine_handle<promise_type>;
            resume_coro get_return_object() { return resume_coro(handle::from_promise(*this)); }
            std::suspend_never initial_suspend() noexcept { return {}; }
            void unhandled_exception() noexcept {}
            void return_void() {}
            std::suspend_never final_suspend() noexcept { return {}; }
        };

        resume_coro(promise_type::handle handle)
        : handle_(handle)
        {}

        ~resume_coro()
        {
            std::cout << "in resume_coro destructor\n";
        }

        promise_type::handle handle_;

        resume_coro(const resume_coro& other) = delete;
        resume_coro& operator=(const resume_coro& other) = delete;
        resume_coro(resume_coro&& other)
        : handle_(other.handle_)
        {
            other.handle_ = nullptr;
        }
        resume_coro& operator=(resume_coro&& other)
        {
            if (this == &other) return *this;
            if (handle_) handle_.destroy();
            std::swap(this->handle_, other.handle_);
            return *this;
        }
    };


    struct resume_awaitable
    {
        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<> h)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            executor_->queue_resume(h);
        }
        void await_resume() {};


        resume_awaitable(std::shared_ptr<CoroExecutor::CoroExecutor> executor)
        : executor_(executor)
        {}

        std::shared_ptr<CoroExecutor::CoroExecutor> executor_;
    };

    struct mock_blocking_with_resume
    {
        bool await_ready() { return false; }
        void await_suspend(CoroExecutor::LifetimeManagedCoroutine::promise_type::handle h)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            std::weak_ptr<CoroExecutor::CoroExecutor> exec = h.promise().executor;
            if (std::shared_ptr<CoroExecutor::CoroExecutor> executor =  exec.lock())
            { 
                executor->queue_deletion(h);
            }
        }
        void await_resume() {};

    };


    resume_coro test_resume(std::promise<void> resume_promise);
    resume_coro test_resume_awaitable(std::promise<void> resume_promise, std::shared_ptr<CoroExecutor::CoroExecutor> executor);
    resume_coro test_thread_id(std::promise<std::thread::id> thread_promise);
    resume_coro test_thread_id_awaitable(std::promise<std::thread::id> thread_promise, std::shared_ptr<CoroExecutor::CoroExecutor> executor);

    CoroExecutor::LifetimeManagedCoroutine test_resume_lifetime(std::promise<void> resume_promise);
    CoroExecutor::LifetimeManagedCoroutine test_complex_coroutine(std::promise<void> resume_promise);
    CoroExecutor::LifetimeManagedCoroutine test_return_with_promise(std::promise<int> prom, int value);
    CoroExecutor::LifetimeManagedCoroutine test_hangs_indefinitely(std::promise<void> resume_promise);

}

using namespace CoroTesting;

class SingleThreadedCoroExecutorTest : public testing::Test
{
protected:
    SingleThreadedCoroExecutorTest()
    {
        executor = CoroExecutor::CoroExecutor::getExecutor(1);
    }
    ~SingleThreadedCoroExecutorTest()
    {
        std::cout << "in fixture destructor\n";
        executor->stop();
    }

public:
        std::shared_ptr<CoroExecutor::CoroExecutor> executor;
};


class MultiThreadedCoroExecutorTest : public testing::Test
{
protected:
    MultiThreadedCoroExecutorTest()
    {
        executor = CoroExecutor::CoroExecutor::getExecutor(4);
    }
    ~MultiThreadedCoroExecutorTest()
    {
        std::cout << "in fixture destructor\n";
        executor->stop();
    }

public:
        std::shared_ptr<CoroExecutor::CoroExecutor> executor;
};

#endif