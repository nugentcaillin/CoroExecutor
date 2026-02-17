#ifndef CORO_EXECUTOR_WORK_ITEM_H
#define CORO_EXECUTOR_WORK_ITEM_H

#include <coroutine>

namespace CoroExecutor
{


struct WorkItem
{
    int priority;
    std::coroutine_handle<> h;
};


} // namespace CoroExecutor
#endif // CORO_EXECUTOR_WORK_ITEM_H