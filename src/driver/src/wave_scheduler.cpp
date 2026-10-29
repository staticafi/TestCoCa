#include <driver/wave_scheduler.hpp>

#include <algorithm>
#include <limits>

namespace {

std::size_t saturating_multiply(std::size_t lhs, std::size_t rhs)
{
    if (lhs != 0 && rhs > std::numeric_limits<std::size_t>::max() / lhs)
        return std::numeric_limits<std::size_t>::max();
    return lhs * rhs;
}

}  // namespace

std::optional<wave_plan> next_wave_plan(const time_budget& budget,
                                        std::size_t wave_index,
                                        std::size_t active_tests)
{
    if (active_tests == 0 || wave_index >= wave_safety_max)
        return std::nullopt;

    const std::size_t usable = budget.usable_ms();
    if (usable < min_useful_slice_ms)
        return std::nullopt;

    std::size_t ladder_slice = wave_min_duration;
    for (std::size_t i = 0; i < wave_index; ++i) {
        ladder_slice = saturating_multiply(ladder_slice,
                                           wave_duration_increase_ratio);
        if (ladder_slice >= usable)
            break;
    }

    const std::size_t fair_share = usable / active_tests;
    const std::size_t timeout = std::min(ladder_slice, fair_share);

    if (timeout < min_useful_slice_ms)
        return std::nullopt;

    return wave_plan{timeout, wave_index};
}
