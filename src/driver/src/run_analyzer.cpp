#include "driver/run_analyzer.hpp"

#include <boost/property_tree/json_parser.hpp>

void run_analyzer::add_execution(connection::shared_memory &src) {

    if (test_type == ERROR_CALL)
    {
        if (src.get_termination() ==
            instrumentation::target_termination::ver_error_reached) {
            goal_reached = true;
        }

        return;
    }

    if (br_instr_count == 0) br_instr_count = src.get_cond_br_count();

    unsigned char *mem = src.get_memory() +
                         sizeof(instrumentation::target_termination) * 2 +
                         sizeof(natural_32_bit);

    for (auto *br_info = reinterpret_cast<instrumentation::br_instr_coverage_info *>(mem);
         br_info->id != 0;
         mem += sizeof(instrumentation::br_instr_coverage_info),
         br_info = reinterpret_cast<instrumentation::br_instr_coverage_info *>(mem))
    {
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


std::pair<double, coverage_map> run_analyzer::get_result() {
    if (test_type == ERROR_CALL)
    {
        return {(double) goal_reached, {}};
    }

    uint64_t conditions_covered = 0;
    for (auto &[id, cc]: coverage) {
        conditions_covered += cc == instrumentation::BOTH ? 2 : 1;
    }

    std::cout << "Total: " << br_instr_count * 2 << std::endl;
    std::cout << "Covered: " << conditions_covered << std::endl;

    if (br_instr_count == 0 || conditions_covered == 0) {
        return {0.0, coverage};
    }

    return {conditions_covered / static_cast<long double>(br_instr_count * 2), coverage};
}