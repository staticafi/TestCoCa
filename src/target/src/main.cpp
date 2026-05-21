#include <sys/prctl.h>
#include <iostream>
#include <target/target.hpp>

extern "C" {
    extern uint32_t __testcoca_cond_br_count __attribute__((weak));
    extern uint32_t __testcoca_goal_count __attribute__((weak));
    void __testcoca_original_main();
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

    target->shared_memory.clear();

    auto* header = target->shared_memory.header();
    header->cond_br_count = &__testcoca_cond_br_count != nullptr ? __testcoca_cond_br_count : (uint32_t) 0;
    header->goal_count = &__testcoca_goal_count != nullptr ? __testcoca_goal_count : (uint32_t) 0;
    header->termination = target_termination::normal;

    header->header_checksum = connection::shared_memory::compute_header_checksum(*header);
    header->data_checksum = 0;

    *target->shared_memory.saved_cursor() = sizeof(connection::shared_memory::metadata_header);

    __testcoca_original_main();

    target->shared_memory.set_termination(target_termination::normal);

    return 0;
}
