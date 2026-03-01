#ifndef CORO_EXECUTOR_UMBRELLA_H
#define CORO_EXECUTOR_UMBRELLA_H

#include <CoroExecutor/Task/Task.hpp>
#include <CoroExecutor/Pool/Umbrella.hpp>
#include <CoroExecutor/ScheduleStrategy/Umbrella.hpp>
#include <CoroExecutor/StorageStrategy/Umbrella.hpp>

// Thread pool and Task<T> type to manage the execution of C++20 coroutines.
// Provides coroutine type that is thread safe, and ensures coroutine
// frame is only destroyed once, and ability to either resume execution
// where data is obtained, or hand off to a worker thread to resume 


#endif // CORO_EXECUTOR_UMBRELLA_H
