#include "driver/run_analyzer.hpp"

#include <boost/property_tree/json_parser.hpp>

void run_analyzer::add_execution(connection::shared_memory &src) {
    if (br_instr_count == 0) br_instr_count = src.get_cond_br_count();

    unsigned char *mem = src.get_memory() +
                         sizeof(instrumentation::target_termination) * 2 +
                         sizeof(natural_32_bit);

    for (auto *br_info = reinterpret_cast<instrumentation::br_instr_coverage_info *>(mem);
         br_info->id != 0;
         mem += sizeof(instrumentation::br_instr_coverage_info),
         br_info = reinterpret_cast<instrumentation::br_instr_coverage_info *>(mem)) {
        if (auto it = coverage.find(br_info->id); it != coverage.end()) {
            if (it->second != instrumentation::BOTH &&
                it->second != br_info->coverage) {
                it->second = instrumentation::BOTH;
            }
        } else {
            coverage.emplace(br_info->id, br_info->coverage);
        }
    }
}


float run_analyzer::get_result() {
    natural_32_bit contitions_covered = 0;
    for (auto &[id, cc]: coverage) {
        contitions_covered += cc == instrumentation::BOTH ? 2 : 1;
    }

    return contitions_covered / static_cast<float>(br_instr_count * 2);
}

void run_analyzer::reset() {
    coverage.clear();
    br_instr_count = 0;
}

