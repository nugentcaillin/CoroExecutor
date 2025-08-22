#include "testing.hpp"
namespace CoroTesting
{
    resume_coro test_resume(std::promise<void> resume_promise) {
        co_await std::suspend_always();
        resume_promise.set_value();
        co_return;
    }

    resume_coro test_resume_awaitable(std::promise<void> resume_promise, std::shared_ptr<CoroExecutor::CoroExecutor> executor)
    {
        co_await resume_awaitable(std::move(executor));
        resume_promise.set_value();
        std::cout << "resumed\n";
        co_return;

    }

    resume_coro test_thread_id(std::promise<std::thread::id> thread_promise)
    {
        co_await std::suspend_always();
        thread_promise.set_value(std::this_thread::get_id());
    }
    
    resume_coro test_thread_id_awaitable(std::promise<std::thread::id> thread_promise, std::shared_ptr<CoroExecutor::CoroExecutor> executor)
    {
        co_await resume_awaitable(std::move(executor));
        thread_promise.set_value(std::this_thread::get_id());
    }

    CoroExecutor::LifetimeManagedCoroutine test_resume_lifetime(std::promise<void> resume_promise)
    {
        std::cout << "resumed\n";
        resume_promise.set_value();
        co_return;
    }

    CoroExecutor::LifetimeManagedCoroutine test_complex_coroutine(std::promise<void> resume_promise)
    {
        co_await mock_blocking_with_resume();
        co_await mock_blocking_with_resume();
        co_await mock_blocking_with_resume();
        co_await mock_blocking_with_resume();
        resume_promise.set_value();
        co_return;
    }
    CoroExecutor::LifetimeManagedCoroutine test_return_with_promise(std::promise<int> prom, int value)
    {
        prom.set_value(value);
        co_return;
    }

    CoroExecutor::LifetimeManagedCoroutine test_hangs_indefinitely(std::promise<void> resume_promise)
    {
        resume_promise.set_value();
        co_await std::suspend_always();
    }
}