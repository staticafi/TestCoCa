#include <instrumentation/data_record_id.hpp>
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

void print_shared_memory(const unsigned char* buffer, size_t size) {
    std::cout << "Shared Memory Dump (" << size << " bytes):\n";

    for (size_t i = 0; i < size; i += 16) {  // Print 16 bytes per row
        std::cout << std::setw(4) << std::setfill('0') << i << "  ";

        // Print hex values
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                std::cout << std::hex << std::setw(2) << std::setfill('0')
                          << (int)buffer[i + j] << " ";
            } else {
                std::cout << "   ";  // Padding for alignment
            }
        }

        std::cout << " ";

        // Print ASCII characters
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                char c = buffer[i + j];
                std::cout << (std::isprint(c) ? c : '.');  // Replace non-printable chars
            }
        }

        std::cout << std::endl;
    }
}

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
    //target->load_stdin();

    target->shared_memory.clear();

    // Reserve the first two bytes for termination
    target->shared_memory << data_record_id::invalid
                                     << data_record_id::invalid;

    __qmi_original_main();

    print_shared_memory(target->shared_memory.get_memory(), 100);

    target->shared_memory.set_termination(
        target_termination::normal);

    return 0;
}
