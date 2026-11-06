#include "connection/shared_memory.hpp"
#include <algorithm>
#include <cassert>
#include <sys/prctl.h>
#include <iostream>
#include <target/target.hpp>

extern "C" {
    extern uint32_t __testcoca_cond_br_count __attribute__((weak));
    extern uint32_t __testcoca_goal_count __attribute__((weak));
    void __testcoca_original_main(int argc, char** argv);
}

namespace instrumentation {
std::unique_ptr<class target> target = std::make_unique<class target>();
}

using namespace instrumentation;


int main(int argc, char* argv[])
{
    /* disable core dumps as this significantly slows down the termination
    in case of a crash */
    if (prctl(PR_SET_DUMPABLE, 0) != 0) {
        std::cerr << "Failed to set dumpable flag\n";
    }

    target->shared_memory.open_or_create();
    target->shared_memory.map_region();

    target->load_config();
    target->load_stdin();

    auto* header = target->shared_memory.header();

    const uint32_t cond_br_count =
        &__testcoca_cond_br_count != nullptr ? __testcoca_cond_br_count : (uint32_t) 0;
    const uint32_t goal_count =
        &__testcoca_goal_count != nullptr ? __testcoca_goal_count : (uint32_t) 0;

    const std::size_t coverage_bytes = std::max<std::size_t>(
        (std::size_t)cond_br_count * sizeof(br_instr_coverage_info),
        (std::size_t)goal_count);

    target->shared_memory.clear_coverage(coverage_bytes);

    header->cond_br_count = cond_br_count;
    header->goal_count = goal_count;
    header->data_checksum = 0;
    header->termination = target_termination::normal;
    header->set_header_checksum();

    *target->shared_memory.saved_cursor() = sizeof(*header);

    __testcoca_original_main(argc, argv);

    return 0;
}
