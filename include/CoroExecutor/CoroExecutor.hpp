#ifndef CORO_EXECUTOR
#define CORO_EXECUTOR

#include <coroutine>
#include <queue>
#include <memory>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>


class CoroExecutorTest;

namespace CoroExecutor
{

class CoroExecutor;





class LifetimeManagedCoroutine
{
    friend class ::CoroExecutorTest;
    friend class CoroExecutor;
public:
    struct final_awaitable;
    struct promise_type 
    {
        using handle  = std::coroutine_handle<promise_type>;
        LifetimeManagedCoroutine get_return_object() { return LifetimeManagedCoroutine(handle::from_promise(*this)); };
        void unhandled_exception() noexcept {};
        std::suspend_always initial_suspend() noexcept { return {}; };
        void return_void() {};

        // signal to CoroExecutor coroutine is ready for deletion
        final_awaitable final_suspend() noexcept;
        
        // assigned by CoroExecutor in add_lifetime_coroutine
        std::shared_ptr<CoroExecutor> executor;
    };

    // behaves as std::suspend_never if no executor, as std::suspend_always if present
    struct final_awaitable {
        bool await_ready() noexcept;

        // queue up handle for deletion
        void await_suspend(promise_type::handle handle) noexcept;
        void await_resume() noexcept {};
    
        std::shared_ptr<CoroExecutor> executor_;
    
        final_awaitable
        ( std::shared_ptr<CoroExecutor> executor
        )
        : executor_(executor)
        {}
    };

    LifetimeManagedCoroutine
    ( promise_type::handle handle
    )
    : handle_(handle)
    {}; 

    // Move only
    LifetimeManagedCoroutine(const LifetimeManagedCoroutine& other) = delete;
    LifetimeManagedCoroutine& operator=(const LifetimeManagedCoroutine& other) = delete;
    LifetimeManagedCoroutine(LifetimeManagedCoroutine&& other)
    : handle_(other.handle_)
    {
        other.handle_ = nullptr;
    }
    LifetimeManagedCoroutine& operator=(LifetimeManagedCoroutine&& other)
    {
        if (this == &other) return *this;
        if (handle_) handle_.destroy();
        handle_ = other.handle_;
        other.handle_ = nullptr;

        return *this;
    }

    ~LifetimeManagedCoroutine()
    {
        // safety guard - this should never evaluate to true if used properly
        if (handle_)
        {
            handle_.destroy();
        }
    }



private:
    promise_type::handle handle_;

};




class CoroExecutor : public std::enable_shared_from_this<CoroExecutor>
{
public:
    // create an instance of CoroExecutor with num_threads threads
    CoroExecutor(int num_threads);

    // queue a coroutine to be resumed on a worker thread
    void queue_resume(std::coroutine_handle<> handle);

    // register a created LifetimeManagedCoroutine 
    void add_lifetime_coroutine(LifetimeManagedCoroutine coro);

    // queues registered LifetimeManagedCoroutine for deletion
    void queue_deletion(LifetimeManagedCoroutine::promise_type::handle handle);

    ~CoroExecutor();
private:

    void worker_loop();

    std::map<LifetimeManagedCoroutine::promise_type::handle, LifetimeManagedCoroutine> lifetime_coros_;
    std::queue<std::coroutine_handle<>> to_resume;
    std::queue<LifetimeManagedCoroutine::promise_type::handle> to_destroy; 

    std::vector<std::thread> worker_threads;
    std::mutex mu;
    std::condition_variable cv;
    bool stop_requested;
};


} // namespace CoroExecutor

#endif // CORO_EXECUTOR