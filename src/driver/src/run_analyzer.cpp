#include "driver/run_analyzer.h"


void run_analyzer::add_execution(connection::shared_memory &src) {
    if (br_instr_count == 0) br_instr_count = src.get_cond_br_count();

    void* mem = src.get_memory() + (sizeof(instrumentation::target_termination) + sizeof(natural_32_bit)) * 2;

    for (;;mem += sizeof(instrumentation::br_instr_coverage_info)) {
        auto* br_info = static_cast<instrumentation::br_instr_coverage_info *>(mem);

        if (!coverage.contains(br_info->id)) {
            coverage[br_info->id] = br_info->coverage;
            continue;
        }

        if (coverage[br_info->id] == instrumentation::BOTH || coverage[br_info->id] == br_info->coverage) {
            continue;
        }

        coverage[br_info->id] = instrumentation::BOTH;
    }
}

float run_analyzer::get_result() {
}
