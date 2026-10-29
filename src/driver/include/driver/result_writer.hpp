#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <target/instrumentation_types.hpp>

using coverage_map = std::unordered_map<instrumentation::br_instr_id, instrumentation::condition_coverage>;

struct test_result
{
    double       score;
    coverage_map branch_coverage;
    bool         time_limit_reached = false;
    std::size_t  elapsed_ms = 0;
    std::size_t  tests_executed = 0;
    std::size_t  tests_skipped = 0;
    std::string  stop_reason;
};

void write_result_json(const std::string& filepath, const test_result& result);
