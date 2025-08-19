#include "CoroExecutor/CoroExecutor.hpp"
#include <iostream>
#include <coroutine>

namespace CoroExecutor
{

bool LifetimeManagedCoroutine::final_awaitable::await_ready() noexcept
{
    return false;
}

void LifetimeManagedCoroutine::final_awaitable::await_suspend(LifetimeManagedCoroutine::promise_type::handle handle) noexcept
{
    if (executor_)
    {
        executor_->queue_deletion(handle);
    }
}


// queue destruction on executor if exists, otherwise destroy handle
LifetimeManagedCoroutine::final_awaitable LifetimeManagedCoroutine::promise_type::final_suspend() noexcept
{
    return final_awaitable(executor);
}




} // namespace CoroExecutor