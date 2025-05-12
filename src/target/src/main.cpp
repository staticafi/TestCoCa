#include <sys/prctl.h>
#include <iostream>
#include <target/target.hpp>

extern "C" {
    extern int __testcoca_cond_br_count;
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

    target->shared_memory << target_termination::normal;
    target->shared_memory << __testcoca_cond_br_count;
    target->shared_memory << (uint64_t) 0; // checksum

    __testcoca_original_main();

    target->shared_memory.set_termination(target_termination::normal);

    return 0;
}
