#pragma once

#include <string>
#include <unordered_map>
#include <target/instrumentation_types.hpp>

using coverage_map = std::unordered_map<instrumentation::br_instr_id, instrumentation::condition_coverage>;

struct test_result
{
    double       score;
    coverage_map branch_coverage;
};

void write_result_json(const std::string& filepath, const test_result& result);
