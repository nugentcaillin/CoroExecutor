#include "CoroExecutor/CoroExecutor.hpp"

namespace CoroExecutor
{

CoroExecutor::CoroExecutor(int num_threads)
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

void CoroExecutor::queue_deletion(LifetimeManagedCoroutine::promise_type::handle)
{

}

CoroExecutor::~CoroExecutor()
{
    stop_requested = true;
    cv.notify_all();
    for (int i { 0 }; i < worker_threads.size(); ++i)
    {
        worker_threads[i].join();
    }
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
            cv.wait(lk, [this] { return stop_requested || !to_resume.empty() || !to_destroy.empty(); });
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
            }
        }

        if (resume_handle)
        {
            resume_handle.resume();
        }
        if (destroy_handle)
        {
            destroy_handle.destroy();
        }
    }
}

} // namespace CoroExecutor