#pragma once

#include <algorithm>
#include <chrono>
#include <cstddef>

class time_budget
{
public:
    using clock = std::chrono::steady_clock;

    explicit time_budget(std::size_t total_ms)
        : start_(clock::now())
        , total_ms_(total_ms)
        , reserve_ms_(compute_reserve(total_ms))
    {
    }

    std::size_t total_ms() const
    {
        return total_ms_;
    }

    std::size_t elapsed_ms() const
    {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            clock::now() - start_).count();
        return elapsed < 0 ? 0 : static_cast<std::size_t>(elapsed);
    }

    std::size_t remaining_ms() const
    {
        const std::size_t elapsed = elapsed_ms();
        return elapsed >= total_ms_ ? 0 : total_ms_ - elapsed;
    }

    std::size_t reserve_ms() const
    {
        return reserve_ms_;
    }

    std::size_t usable_ms() const
    {
        const std::size_t remaining = remaining_ms();
        return remaining > reserve_ms_ ? remaining - reserve_ms_ : 0;
    }

    bool exhausted(std::size_t min_slice_ms = 0) const
    {
        return usable_ms() < min_slice_ms;
    }

private:
    static std::size_t compute_reserve(std::size_t total_ms)
    {
        if (total_ms == 0)
            return 0;

        const std::size_t five_percent = total_ms / 20;
        const std::size_t clamped =
            std::clamp<std::size_t>(five_percent, 20, 500);

        return std::min(clamped, total_ms / 10);
    }

    clock::time_point start_;
    std::size_t total_ms_;
    std::size_t reserve_ms_;
};
