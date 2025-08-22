#include "CoroExecutor/CoroExecutor.hpp"
#include <iostream>
#include <cassert>

namespace CoroExecutor
{

CoroExecutor::CoroExecutor()
: lifetime_coros_()
, to_resume()
, to_destroy()
, worker_threads()
, mu()
, cv()
, stop_requested(false)
{
    std::cout << "Inside CoroExecutor constructor\n";
    /*
    for (int i { 0 }; i < num_threads; ++i)
    {
        worker_threads.push_back(std::thread([this] { worker_loop(); }));
    }
    */
}

void CoroExecutor::start(int num_threads)
{
    for (int i { 0 }; i < num_threads; ++i)
    {
        worker_threads.push_back(std::thread([this] { worker_loop(); }));
    }
}

void CoroExecutor::queue_resume(std::coroutine_handle<> handle)
{
    {
        std::lock_guard<std::mutex> lk(mu);
        to_resume.push(handle);
    }
    cv.notify_one();
}

void CoroExecutor::queue_deletion(LifetimeManagedCoroutine::promise_type::handle handle)
{
    {
        std::lock_guard<std::mutex> lk(mu);
        to_destroy.push(handle);
    }
    cv.notify_one();
}


void CoroExecutor::add_lifetime_coroutine(LifetimeManagedCoroutine coro)
{
    coro.handle_.promise().executor = shared_from_this();
    LifetimeManagedCoroutine::promise_type::handle handle = coro.handle_;
    {
        std::lock_guard<std::mutex> lk(mu);
        lifetime_coros_.emplace(handle, std::move(coro));
    }
    queue_resume(handle);
}

void CoroExecutor::stop()
{
    {
        std::lock_guard lk(mu);
        stop_requested = true;
    }
    cv.notify_all();
    for (unsigned int i { 0 }; i < worker_threads.size(); ++i)
    {   
        if (!worker_threads[i].joinable()) continue;
        if (std::this_thread::get_id() == worker_threads[i].get_id())
        {
            std::cerr << "Cannot destroy from worker thread. See pitfalls > self join in readme for more info\n";
            std::terminate();
        }
        worker_threads[i].join();
    }
}

CoroExecutor::~CoroExecutor()
{
    stop();
}

void CoroExecutor::worker_loop()
{
    while (true)
    {
        std::coroutine_handle<> resume_handle {};
        LifetimeManagedCoroutine::promise_type::handle destroy_handle {};

        // acquire handle, exit loop if no handles to process and stop requested
        {
            std::unique_lock lk(mu);
            std::cout << "waiting on cv\n";
            cv.wait(lk, [this] {
            std::cout << "checking predicate\n"; 
            return stop_requested || !to_resume.empty() || !to_destroy.empty(); });
            if (stop_requested && to_resume.empty() && to_destroy.empty()) return;
            
            // prioritising resuming over destroying
            if (!to_resume.empty())
            {
                resume_handle = to_resume.front();
                to_resume.pop();
            } else if (!to_destroy.empty())
            {
                destroy_handle = to_destroy.front();
                to_destroy.pop();
                lifetime_coros_.erase(destroy_handle);
            }
        } 

        if (resume_handle)
        {
            resume_handle.resume();
        }
    }
}

} // namespace CoroExecutor