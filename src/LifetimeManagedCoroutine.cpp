#include "CoroExecutor/CoroExecutor.hpp"
#include <iostream>
#include <coroutine>

namespace CoroExecutor
{

bool final_awaitable::await_ready() noexcept
{
    return !executor_;
}

void final_awaitable::await_suspend(std::coroutine_handle<>) noexcept
{
    // guaranteed to have executor to be valid if we entered await_suspend

    // queue up handle for deletion
}


// queue destruction on executor if exists, otherwise destroy handle
final_awaitable LifetimeManagedCoroutine::promise_type::final_suspend() noexcept
{
    return final_awaitable(executor);
}

void LifetimeManagedCoroutine::destroy_self()
{
    if (handle_.promise().executor)
    {
        // enrol coroutine for destruction
    } else 
    {
        handle_.destroy();
    }
}

} // namespace CoroExecutor