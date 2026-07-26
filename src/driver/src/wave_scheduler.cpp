#include <driver/wave_scheduler.hpp>

std::vector<size_t> calculate_waves(size_t limit, size_t test_count)
{
    if (test_count == 0) return {};

    if (test_count == 1) return {limit};

    if (limit / test_count < wave_min_duration) {
        return {wave_min_duration};
    }

    std::vector<size_t> waves;
    const size_t r = wave_duration_increase_ratio;
    size_t remaining = limit;
    size_t current = wave_min_duration;

    while (waves.size() < wave_max_count) {
        const size_t wave_time = current;
        const size_t total_consumed = wave_time * test_count;

        if (total_consumed > remaining) {
            break;
        }

        waves.push_back(wave_time);
        remaining -= total_consumed;
        current *= r;
    }

    if (!waves.empty() && remaining > 0) {
        waves.back() += remaining / test_count;
    }

    if (waves.empty()) waves.push_back(wave_min_duration);

    return waves;
}
