#ifndef CORO_EXECUTOR_TASK_H
#define CORO_EXECUTOR_TASK_H


#include <coroutine>
#include <exception>
#include <iostream>
#include <atomic>
#include <utility>
#include <optional>
#include <stdexcept>
#include <vector>
#include <mutex>

namespace CoroExecutor
{

template<typename T>
struct Task
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type
    {
        std::optional<T> value_;
        std::exception_ptr exception_;
        std::atomic<int> ref_count;
        std::vector<std::coroutine_handle<>> to_resume;
        std::mutex to_resume_mutex;

        Task get_return_object() { return Task(handle_type::from_promise(*this)); }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_value(T value) { 
            value_ = value;
            std::lock_guard<std::mutex> guard(to_resume_mutex);
            for (std::coroutine_handle<> handle: to_resume) handle.resume();
        }
        void unhandled_exception () { exception_ = std::current_exception(); }
        promise_type()
        : value_ {}
        , exception_ { nullptr }
        , ref_count { 0 }
        , to_resume {}
        , to_resume_mutex {}
        {
            to_resume.reserve(4);
        }
    };

    // awaitable to get product from co_await
    struct awaiter
    {
        Task to_await_;
        awaiter(Task to_await)
        : to_await_(to_await) // copy so this task lives until result is consumed
        {}
        bool await_ready() { return to_await_.handle_.promise().value_.has_value(); }
        bool await_suspend(std::coroutine_handle<> to_resume)
        {
            std::lock_guard<std::mutex> guard(to_await_.handle_.promise().to_resume_mutex);
            if (await_ready()) return false; // ensure we do not add handles to be resumed if return_value already of task already called
            to_await_.handle_.promise().to_resume.push_back(to_resume);
            return true;
        }
        T await_resume() 
        {
            // only enter this value if we have a value, or coroutine needs to throw exception,
            // this runs on thread that resumes coroutine, so safe to throw
            if (await_ready()) return to_await_.handle_.promise().value_.value();
            std::exception_ptr exception = to_await_.handle_.promise().exception_;
            if (!exception) throw(new std::logic_error("resumed Task return awaitable without value or exception"));
            std::rethrow_exception(exception); 
        }

    };

    awaiter operator co_await() { return awaiter(*this); }


    Task(handle_type handle)
    : handle_ {handle}
    {
        if (handle_) handle_.promise().ref_count.fetch_add(1);
    }
    Task()
    : Task(nullptr)
    {}

    Task(const Task& other) : Task(other.handle_) {}
    Task& operator=(const Task& other)
    {
        if (this == &other) return *this;
        // destroy this handle if this is unique owner
        if (handle_ && handle_.promise().ref_count.fetch_sub(1) == 1) handle_.destroy();
        handle_ = other.handle_;
        if (handle_) handle_.promise().ref_count.fetch_add(1); 
        return *this;
    }

    Task(Task&& other) noexcept
    : handle_(std::exchange(other.handle_, nullptr))
    {}

    Task& operator=(Task&& other) noexcept
    {
        if (this == &other) return *this;
        // destroy this handle if this is unique owner
        if (handle_ && handle_.promise().ref_count.fetch_sub(1) == 1) handle_.destroy();
        handle_ = std::exchange(other.handle_, nullptr);
        return *this;
    }


    ~Task()
    {
        if (!handle_) return;
        // destroy this handle if this is unique owner
        if (handle_ && handle_.promise().ref_count.fetch_sub(1) == 1) handle_.destroy();
        handle_ = nullptr; // only relevant if destructor called multiple times for same object
    }

    std::coroutine_handle<Task::promise_type> handle_;
};


} // namespace CoroExecutor
#endif // CORO_EXECUTOR_TASK_H 