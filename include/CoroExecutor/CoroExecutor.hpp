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

namespace CoroExecutor
{

class CoroExecutor
{
public:
    // create an instance of CoroExecutor with num_threads threads
    CoroExecutor(int num_threads);

    // queue a coroutine to be resumed on a worker thread
    void resume_coroutine(std::coroutine_handle<> handle);

    // register a created LifetimeManagedCoroutine 
    void add_lifetime_coroutine(LifetimeManagedCoroutine coro);

    ~CoroExecutor();
private:
    void delete_lifetime_coroutine(LifetimeManagedCoroutine::promise_type::handle handle);
    std::map<LifetimeManagedCoroutine::promise_type::handle, LifetimeManagedCoroutine> lifetime_coros_;
    std::queue<std::coroutine_handle<>> to_resume;
    std::queue<LifetimeManagedCoroutine::promise_type::handle> destruction_queue; 

    std::vector<std::thread> worker_threads;
    std::mutex mu;
    std::condition_variable cv;
    bool stop_requested;
};


class LifetimeManagedCoroutine
{
public:
    struct promise_type 
    {
        using handle  = std::coroutine_handle<promise_type>;
        LifetimeManagedCoroutine get_return_object() { return LifetimeManagedCoroutine(handle::from_promise(*this)); };
        void unhandled_exception() noexcept {};
        std::suspend_always initial_suspend() noexcept {};
        void return_void() {};

        // signal to CoroExecutor coroutine is ready for deletion
        std::suspend_always final_suspend();
        
        // assigned by CoroExecutor in add_lifetime_coroutine
        std::shared_ptr<CoroExecutor> executor;
    };


    LifetimeManagedCoroutine
    ( promise_type::handle handle
    )
    : handle_(handle)
    {}; 

private:
    promise_type::handle handle_;
};


} // namespace CoroExecutor

#endif // CORO_EXECUTOR