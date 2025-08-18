#ifndef TESTING_H
#define TESTING_H

#include <gtest/gtest.h>
#include <CoroExecutor/CoroExecutor.hpp>
#include <latch>
#include <thread>
#include <atomic>
#include <coroutine>


class CoroExecutorTest : public testing::Test
{
public:

    struct queue_resume_coro {
        struct promise_type {
            using handle = std::coroutine_handle<promise_type>;
            queue_resume_coro get_return_object() { return queue_resume_coro(handle::from_promise(*this)); }
            void unhandled_exception() noexcept {}
            std::suspend_never initial_suspend() noexcept { return {}; }
            void return_void() {};
            std::suspend_never final_suspend() noexcept { return {}; }
        };
        
        queue_resume_coro(promise_type::handle handle)
        : handle_(handle)
        {}

        promise_type::handle handle_;
    };

    class DestructionTracker {
    public:
        DestructionTracker(std::atomic<int>& destruction_counter)
        : destruction_counter_(destruction_counter)
        {}
        ~DestructionTracker()
        {
            destruction_counter_.fetch_add(1);
        }
    private:
        std::atomic<int>& destruction_counter_;
    };

    struct queue_resume
    {
        bool await_ready() noexcept { return false; }
        void await_suspend(std::coroutine_handle<> h) noexcept 
        {
            exec_->queue_resume(h);
        }
        void await_resume() noexcept {}

        queue_resume(std::shared_ptr<CoroExecutor::CoroExecutor> exec)
        : exec_(exec)
        {}

        std::shared_ptr<CoroExecutor::CoroExecutor> exec_;
    };

    CoroExecutor::LifetimeManagedCoroutine test_coro(std::latch& latch, std::atomic<int>& counter, std::thread::id& id) {
        counter.fetch_add(1);
        id = std::this_thread::get_id();
        latch.count_down();
        co_return;
    }

    queue_resume_coro queue_resumption_with_awaitable(std::latch& latch, std::atomic<int>& counter, std::thread::id& id, std::shared_ptr<CoroExecutor::CoroExecutor> exec)
    {
        co_await queue_resume(exec);
        latch.count_down();
        counter.fetch_add(1);
        id = std::this_thread::get_id();
        co_return;
    }

    queue_resume_coro count_destruction_with_awaitable(std::latch& latch, std::atomic<int>& destruction_counter, std::shared_ptr<CoroExecutor::CoroExecutor> exec)
    {
        co_await queue_resume(exec);
        latch.count_down();
        
        DestructionTracker tracker(destruction_counter);
        
        co_return;
    }

    CoroExecutor::LifetimeManagedCoroutine simple_lifetime_coroutine(std::latch& latch, std::atomic<int>& resume_counter, std::atomic<int>& destruction_counter)
    {
        DestructionTracker tracker(destruction_counter);
        resume_counter.fetch_add(1);
        latch.count_down();
        co_return;
    }



    std::coroutine_handle<> handle_helper(CoroExecutor::LifetimeManagedCoroutine& coro) 
    {
        std::coroutine_handle<> handle = coro.handle_;
        coro.handle_ = nullptr;
        return handle;
    }
};

#endif