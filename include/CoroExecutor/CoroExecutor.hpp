#ifndef CORO_EXECUTOR_H
#define CORO_EXECUTOR_H

#include <coroutine>
#include <exception>
#include <iostream>
#include <atomic>
#include <utility>
#include <optional>
#include <stdexcept>

// thread pool to manage lifecycle of coroutines
// it needs:
// 1. Class for scheduling the coroutines
// 2. Class for the managed coroutine

// coroutine manager needs:
// ESSENTIALS:
// 1. Creation of threads
// 2. Destruction of threads
// 3. Main loop for threads
// 4. Ability to Queue Tasks
//     - through an awaitable ? or a method in executor or both
// 5. Ability to queue destruction of coroutines
// 6. Destruction of all managed coroutines in destructor
// NICE TO HAVES:
// 1. Priority system 
// 2. work stealing
// 3. strategy pattern for next task e.g FIFO, LIFO or even composite for main strat then work steal
// 4. support for eagerly executed Tasks

// Task<T>
// ESSENTIALS
// 1. requests own destruction only when last reference
// 2. way to co-await and have executor resume when ready
//    - can either resume where awaited async call happens, or 
// 3. support to synchronously block for result or co_await


// THOUGHTS:
// - executor should only resume and not poll - queue only contains coroutines ready to be resumed
// - should enforce access to only be shared pointer - factory method,
//   can use this factory to give some defaults i.e. default strategy for next task to resume 

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
        std::coroutine_handle<> to_resume;

        Task get_return_object() { return Task(handle_type::from_promise(*this)); }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_value(T value) { 
            std::cout << "return value called with arg: " << value << "\n";
            value_ = value;
            if (to_resume) to_resume.resume(); 
        }
        void unhandled_exception () { exception_ = std::current_exception(); }
        promise_type()
        : value_ {}
        , exception_ { nullptr }
        , ref_count { 0 }
        , to_resume { nullptr }
        {}
    };

    // awaitable to get product from co_await
    struct awaiter
    {
        bool ready_;
        Task to_await_;
        awaiter(bool ready, Task to_await)
        : ready_ { ready }
        , to_await_(to_await) // copy so this task lives until result is consumed
        {}
        bool await_ready() { return ready_; }
        void await_suspend(std::coroutine_handle<> to_resume)
        {
            to_await_.handle_.promise().to_resume = to_resume;
        }
        T await_resume() 
        {
            // only enter this value if we have a value, or coroutine needs to throw exception,
            // this runs on thread that resumes coroutine, so safe to throw
            if (to_await_.handle_.promise().value_) return to_await_.handle_.promise().value_.value();
            std::exception_ptr exception = to_await_.handle_.promise().exception_;
            if (!exception) throw(new std::logic_error("resumed Task return awaitable without value or exception"));
            std::rethrow_exception(exception); 
        }

    };

    awaiter operator co_await() { 
        std::cout << "in co await, should return instantly: " << handle_.promise().value_.has_value() << "\n";
        if (handle_.promise().value_.has_value()) std::cout << handle_.promise().value_.value() << "\n";
        return awaiter(handle_.promise().value_.has_value(), *this); }


    Task(handle_type handle)
    : handle_ {handle}
    {
        if (handle_) handle_.promise().ref_count.fetch_add(1);
    }

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
    }

    std::coroutine_handle<Task::promise_type> handle_;
};



class CoroExecutor
{
public:
private:
    CoroExecutor(unsigned int num_threads);
};



}

#endif // CORO_EXECUTOR_H