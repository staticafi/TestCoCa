#include <boost/process.hpp>
#include <connection/target_executor.hpp>
#include <iostream>

namespace bp = boost::process;
using namespace instrumentation;

namespace connection {

/* boost process wait_for waits for the full duration if the process exited
before wait_for (https://github.com/boostorg/process/issues/69)
the wrapper is a workaround for this issue */
template <typename Rep, typename Period>
static bool wait_for_wrapper(bp::child& process,
                             const std::chrono::duration<Rep, Period>& rel_time)
{
    if (process.running()) {
        return process.wait_for(rel_time);
    }
    return true;
}

target_executor::target_executor(std::string target_invocation)
    : timeout_ms{0}, target_invocation(std::move(target_invocation)), shm{}
{
}

void target_executor::init_shared_memory(std::size_t size)
{
    get_shared_memory().open_or_create();
    get_shared_memory().set_size((natural_32_bit)size);
    get_shared_memory().map_region();
}


void target_executor::execute_target()
{
    try {
        std::cout << "Starting test process..." << std::endl;

        bp::child test_proc(target_invocation, bp::std_out > stdout);

        test_proc.wait();  // Wait for process to finish

        std::cout << "Exit code: " << test_proc.exit_code() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

}

}  // namespace connection
