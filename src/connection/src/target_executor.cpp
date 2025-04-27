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
    get_shared_memory().set_size(size);
    get_shared_memory().map_region();
}

void target_executor::set_timeout(natural_16_bit const timeout_ms_)
{
    timeout_ms = timeout_ms_;
}


void target_executor::execute_target()
{
    using namespace std::chrono_literals;
    bp::child target = bp::child(target_invocation, bp::std_out > stdout, bp::std_err > stderr);
    if (!wait_for_wrapper(target, std::chrono::milliseconds(timeout_ms))) {
        target.terminate();

        std::cout << "TIME OUT: " << timeout_ms <<  std::endl;

        get_shared_memory().set_termination(target_termination::timeout);
    }

    if (!get_shared_memory().get_termination()) {
        if (target.exit_code() == 0) {
            get_shared_memory().set_termination(target_termination::normal);
        }
        else {
            get_shared_memory().set_termination(target_termination::crash);
        }
    }
}

}  // namespace connection
