#pragma once

#include <driver/time_budget.hpp>

#include <cstddef>
#include <optional>

constexpr std::size_t wave_min_duration = 100;
constexpr std::size_t wave_duration_increase_ratio = 5;
constexpr std::size_t min_useful_slice_ms = 5;
constexpr std::size_t wave_safety_max = 64;

struct wave_plan
{
    std::size_t timeout_ms;
    std::size_t wave_index;
};

std::optional<wave_plan> next_wave_plan(const time_budget& budget,
                                        std::size_t wave_index,
                                        std::size_t active_tests);
