#ifndef CORO_EXECUTOR_H
#define CORO_EXECUTOR_H

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

namespace CoroExecutor
{

}

#endif // CORO_EXECUTOR_H