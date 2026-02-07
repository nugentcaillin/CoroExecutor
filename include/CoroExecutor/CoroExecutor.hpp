#ifndef CORO_EXECUTOR_UMBRELLA_H
#define CORO_EXECUTOR_UMBRELLA_H

#include <CoroExecutor/Task/Task.hpp>
#include <CoroExecutor/Pool/Pool.hpp>

// Thread pool and Task<T> type to manage the execution of C++20 coroutines.
// Provides coroutine type that is thread safe, and ensures coroutine
// frame is only destroyed once, and ability to either resume execution
// where data is obtained, or hand off to a worker thread to resume 

// TODO:
// - Implement interface for Pool
// - allow multiple coroutines to await a single task, currently is not thread safe if multiple await - 
//   will overwrite coroutine handle to resume
// - add tests for exception throwing of Task

// NICE TO HAVES:
// - Priority system 
// - work stealing
// - strategy pattern for next task e.g FIFO, LIFO or even composite for main strat then work steal
// - support for eagerly executed Tasks
// - factory method for pool - maybe returning shared pointer? 
// - blocking get method for Task

#endif // CORO_EXECUTOR_UMBRELLA_H