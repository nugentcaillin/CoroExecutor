#include <CoroExecutor/ScheduleStrategy/RoundRobinStrategy.hpp>
#include <stdexcept>

namespace CoroExecutor
{


RoundRobinStrategy::RoundRobinStrategy(size_t num_threads)
: next_thread_(0)
, num_threads_ { num_threads }
{
    if (num_threads == 0) throw std::invalid_argument("Min number of threads is one");
}


size_t RoundRobinStrategy::get_target_thread_id()
{
    size_t next = next_thread_.fetch_add(1);
    return next % num_threads_;
}


} // namespace CoroExecutor

