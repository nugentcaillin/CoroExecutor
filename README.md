# Installation

This library can be made available using FetchContent as follows in your CMakeLists.txt
```console
include(FetchContent)
FetchContent_Declare(
    CoroExecutor
    GIT_REPOSITORY https://github.com/nugentcaillin/CoroExecutor.git
    GIT_TAG e5198c745066d6abf96002ee8c9b212de7abf9b8
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