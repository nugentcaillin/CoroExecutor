#ifndef CORO_EXECUTOR_H
#define CORO_EXECUTOR_H

#include <coroutine>

// thread pool to manage lifecycle of coroutines
// it needs:
// 1. Class for containing the coroutines
// 2. Class for the managed coroutine

// coroutine manager needs:
// ESSENTIALS:
// 1. Creation of threads
// 2. Destruction of threads
// 3. Main loop for threads
// 4. Ability to Queue coroutines
// 5. Ability to queue destruction of coroutines
// 6. Destruction of all managed coroutines in destructor
// NICE TO HAVES:
// 1. Priority system 

// managed coroutine needs
// ESSENTIALS
// 1. requests own destruction only on completion
// 2. way to co-await and have executor resume when ready


// THOUGHTS:
// - executor only needs to store handles since stack is heap allocated, can use coroutine handle as identifier, but allow return object too
// - coroutine could be void with only promises/futures for value returns
// CoroExecutor should enforce access to only be shared pointer

namespace CoroExecutor
{

struct MangagedCoroutine
{
    struct promise_type
    {

        MangagedCoroutine get_return_object() { return MangagedCoroutine(std::coroutine_handle<MangagedCoroutine::promise_type>::from_promise(*this)); }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception () {}
    };

    MangagedCoroutine(std::coroutine_handle<MangagedCoroutine::promise_type> handle)
    : handle_ {handle}
    {}

    std::coroutine_handle<MangagedCoroutine::promise_type> handle_;
};



class CoroExecutor
{
public:
    void add_coroutine(MangagedCoroutine& coro);
    void add_coroutine(std::coroutine_handle<MangagedCoroutine::promise_type> handle);
    void destroy_coroutine(MangagedCoroutine& coro);
    void destroy_coroutine(std::coroutine_handle<MangagedCoroutine::promise_type> handle);
private:
    CoroExecutor(unsigned int num_threads);
};



}

#endif // CORO_EXECUTOR_H