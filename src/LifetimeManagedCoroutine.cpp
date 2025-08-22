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
    if (std::shared_ptr<CoroExecutor> executor =  executor_.lock())
    {
        executor->queue_deletion(handle);
    }
}


// queue destruction on executor if exists, otherwise destroy handle
LifetimeManagedCoroutine::final_awaitable LifetimeManagedCoroutine::promise_type::final_suspend() noexcept
{
    return final_awaitable(executor);
}




} // namespace CoroExecutor