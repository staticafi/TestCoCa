#pragma once

#include <vector>
#include <cstddef>

constexpr size_t wave_min_duration = 100;
constexpr size_t wave_max_count = 4;
constexpr size_t wave_duration_increase_ratio = 5;

std::vector<size_t> calculate_waves(size_t time_limit_ms, size_t test_count);
