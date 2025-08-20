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
CoroExecutor::CoroexEcutor exec(num_threads);
```
where num_threads is the number of threads.

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
