#include <CoroExecutor/ScheduleStrategy/RoundRobinStrategy.hpp>

namespace CoroExecutor
{


RoundRobinStrategy::RoundRobinStrategy(size_t num_threads)
: num_threads_ { num_threads }
, next_thread_(0)
{}

size_t RoundRobinStrategy::get_target_thread_id()
{
    size_t next = next_thread_.fetch_add(1);
    return next % num_threads_;
}


} // namespace CoroExecutor

