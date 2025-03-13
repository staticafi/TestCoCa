#include <instrumentation/target.hpp>
#include <iostream>
#include <iomanip>

extern "C" {
void __qmi_original_main();
}

namespace instrumentation {
std::unique_ptr<class target> target =
    std::make_unique<class target>();
}

using namespace instrumentation;

#if PLATFORM() == PLATFORM_LINUX()
#include <sys/prctl.h>
#endif


int main(int argc, char* argv[])
{

/* disable core dumps as this significantly slows down the termination
in case of a crash */
#if PLATFORM() == PLATFORM_LINUX()
    if (prctl(PR_SET_DUMPABLE, 0) != 0) {
        std::cerr << "Failed to set dumpable flag\n";
    }
#endif

    target->shared_memory.open_or_create();
    target->shared_memory.map_region();

    target->load_config();
    target->load_stdin();
    target->load_stdout();

    target->shared_memory.clear();

    // reserve first byte for target termination
    target->shared_memory << target_termination::normal;

    __qmi_original_main();

    target->shared_memory.set_termination(
        target_termination::normal);

    return 0;
}
