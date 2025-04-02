#pragma once

#include <unordered_map>

#include "connection/shared_memory.hpp"
#include "target/instrumentation_types.hpp"

using coverage_map = std::unordered_map<instrumentation::br_instr_id, instrumentation::condition_coverage>;

class run_analyzer {
    coverage_map coverage;
    natural_32_bit br_instr_count = 0;

   public:
    void add_execution(connection::shared_memory& src);
    std::pair<double, coverage_map> get_result();
    void reset();
};
