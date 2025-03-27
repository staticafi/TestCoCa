#pragma once

#include <unordered_map>

#include "connection/shared_memory.hpp"
#include "target/instrumentation_types.hpp"


class run_analyzer {
    std::unordered_map<instrumentation::br_instr_id, instrumentation::condition_coverage> coverage;
    natural_32_bit br_instr_count = 0;

   public:
    void add_execution(connection::shared_memory& src);
    float get_result();
    void reset();
};
