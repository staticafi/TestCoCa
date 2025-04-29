#pragma once

#include <unordered_map>

#include "connection/shared_memory.hpp"
#include "target/instrumentation_types.hpp"

enum TestType {
    BRANCH_COVERAGE,
    ERROR_CALL
};

using coverage_map = std::unordered_map<instrumentation::br_instr_id, instrumentation::condition_coverage>;

class run_analyzer {
    TestType test_type;
    bool goal_reached = false;
    coverage_map coverage;
    uint64_t br_instr_count = 0;

   public:
    explicit run_analyzer(TestType test_type): test_type(test_type) {}

    void add_execution(connection::shared_memory& src);
    std::pair<double, coverage_map> get_result();
    void reset();
};

