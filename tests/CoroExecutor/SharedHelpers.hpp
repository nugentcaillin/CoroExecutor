#ifndef CORO_EXECUTOR_SHARED_HELPERS_H
#define CORO_EXECUTOR_SHARED_HELPERS_H

#include <vector>
#include <thread>
#include <ranges>

class AttemptRaceCondition
{
protected:
    std::vector<std::thread> threads_;
    virtual void attempt_one() = 0;

    // call functions all to nearest 100ms to attempt race condition
    void synchronize_one()
    {
        auto now = std::chrono::steady_clock::now();
        using ms100 = std::chrono::duration<double, std::ratio<1, 10>>;
        auto resume_point = std::chrono::ceil<ms100>(now);
        std::this_thread::sleep_until(resume_point);
        attempt_one();
    }

    void attempt_many(size_t num_threads)
    {
        join_all();
        for (size_t i : std::ranges::views::iota(0u, num_threads))
        {
            if (i < threads_.size()) threads_.at(i) = std::thread([this]() { this->synchronize_one(); }); 
            else threads_.push_back(std::thread([this]() { this->synchronize_one(); }));
        }
    }
    void join_all()
    {
        for (auto& t : threads_)
        {
            if (t.joinable()) t.join();
        }
    }
    ~AttemptRaceCondition()
    {
        join_all();
    }
};

#endif // CORO_EXECUTOR_SHARED_HELPERS_H