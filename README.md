# About
This library provides an executor to resume coroutines on its own thread pool.

It also provides a coroutine type that can have its lifetime managed by this executor.

This type only returns void, so it is expected that if a user needs a value from this coroutine
promises will be used

# Installation

This library can be made available using FetchContent as follows in your CMakeLists.txt
```console
include(FetchContent)
FetchContent_Declare(
    CoroExecutor
    GIT_REPOSITORY https://github.com/nugentcaillin/CoroExecutor.git
    GIT_TAG 4998274c7377247ae96196c70ffbd21a02baf7e7 # v1.0.0
)
FetchContent_MakeAvailable(CoroExecutor)

target_link_libraries(${PROJECT_NAME} PRIVATE CoroExecutor)
```

# Usage

### Including
This library has one public header which can be included with
```C++
#include <CoroExecutor/CoroExecutor.hpp>
```

The CoroExecutor can be instantiated as follows

```console
std::shared_ptr<CoroExecutor::CoroexEcutor> CoroExecutor::getExecutor(num_threads);
```
where num_threads is the number of threads.

### Cleanup
Threads can be joined at a controlled time using CoroExecutor's ```CoroExecutor::stop()``` method.
It's advised to do this so that threads are joined before the reference to the shared pointer falls out of scope as
this introduces the possibility that a worker thread could call CoroExecutor's destructor

### Resuming

Coroutines can then be resumed inside of awaitable's await_resume by calling CoroExecutor's ```resume_coroutine``` method.

The coroutine will now be resumed inside of one of CoroExecutor's worker threads.

### Managing Lifetime

If you wish to have CoroExecutor manage the lifetime of coroutines, you can leverage its ```add_lifetime_coroutine(LifetimeManagedCoroutine)``` method with a ```LifetimeManagedCoroutine``` as follows:

```C++
LifetimeManagedCoroutine coro()
{
    // coroutine logic
}
auto c = coro();
exec.add_lifetime_coroutine(std::move(c));
```

the lifetime of c will now be managed by CoroExecutor

# Pitfalls

### self join
When using the ad-hoc resume and you're queueing up resuming with awaitables,
it's important to make sure that the coroutine frame doesn't end up with the last reference
to the shared pointer. For example, with this final suspend std::suspend_never fire-and-forget coroutine 
```
// could end up with last reference
resume_coro bad_resume_awaitable(std::promise<void> resume_promise, std::shared_ptr<CoroExecutor::CoroExecutor> executor)
{
    co_await resume_awaitable(std::move(executor));
    resume_promise.set_value();
    std::cout << "resumed\n";
    co_return;
}

// good awaitable
resume_coro test_resume_awaitable(std::promise<void> resume_promise, std::shared_ptr<CoroExecutor::CoroExecutor> executor)
    {
    // shared pointer does not live past this co_await as resume_awaitable is destroyed immediately
    co_await resume_awaitable(std::move(executor));
    resume_promise.set_value();
    std::cout << "resumed\n";
    co_return;
}

```
bad_resume_awaitable could end up with the last copy of the shared pointer, and try to join its own thread to itself.


This can also happen when CoroExecutor's stop() method is not used.
