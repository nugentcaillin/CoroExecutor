#ifndef CORO_EXECUTOR_UMBRELLA_H
#define CORO_EXECUTOR_UMBRELLA_H

#include <CoroExecutor/Task/Task.hpp>
#include <CoroExecutor/Pool/Pool.hpp>

// Thread pool and Task<T> type to manage the execution of C++20 coroutines.
// Provides coroutine type that is thread safe, and ensures coroutine
// frame is only destroyed once, and ability to either resume execution
// where data is obtained, or hand off to a worker thread to resume 

// TODO:
// allow multiple coroutines to await a single task, currently is not thread safe if multiple await
// add tests for exception throwing of Task
// Implement interface for Pool


// Planning:
// thread pool to manage lifecycle of coroutines
// it needs:
// 1. Class for scheduling the coroutines
// 2. Class for the managed coroutine

// coroutine manager needs:
// ESSENTIALS:
// 1. Creation of threads
// 2. Destruction of threads
// 3. Main loop for threads
// 4. Ability to Queue Tasks
//     - through an awaitable ? or a method in executor or both
// 5. Ability to queue destruction of coroutines
// 6. Destruction of all managed coroutines in destructor
// NICE TO HAVES:
// 1. Priority system 
// 2. work stealing
// 3. strategy pattern for next task e.g FIFO, LIFO or even composite for main strat then work steal
// 4. support for eagerly executed Tasks

// Task<T>
// ESSENTIALS
// 1. requests own destruction only when last reference
// 2. way to co-await and have executor resume when ready
//    - can either resume where awaited async call happens, or 
// 3. support to synchronously block for result or co_await


// THOUGHTS:
// - executor should only resume and not poll - queue only contains coroutines ready to be resumed
// - should enforce access to only be shared pointer - factory method,
//   can use this factory to give some defaults i.e. default strategy for next task to resume 


#endif // CORO_EXECUTOR_UMBRELLA_H