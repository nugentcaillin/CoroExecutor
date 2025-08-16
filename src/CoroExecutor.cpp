#include "CoroExecutor/CoroExecutor.hpp"

namespace CoroExecutor
{

CoroExecutor::CoroExecutor(int num_threads)
{

}

void CoroExecutor::queue_resume(std::coroutine_handle<> handle)
{

}

void CoroExecutor::queue_deletion(LifetimeManagedCoroutine::promise_type::handle)
{

}

CoroExecutor::~CoroExecutor()
{

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