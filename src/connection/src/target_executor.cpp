#include <boost/process.hpp>
#include <connection/target_executor.hpp>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>

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
    get_shared_memory().clear();
}

void target_executor::set_timeout(std::size_t const timeout_ms_)
{
    timeout_ms = timeout_ms_;
}

execution_result target_executor::execute_target(
    const std::vector<std::string>& args,
    const std::optional<std::string>& stdin_data)
{
    using namespace std::chrono_literals;

    execution_result result;
    const auto wall_start = std::chrono::steady_clock::now();

    try {
        bp::group group;
        bp::child target;

        if (stdin_data) {
            std::unique_ptr<FILE, decltype(&std::fclose)> stdin_file(
                std::tmpfile(), &std::fclose);
            if (!stdin_file) {
                throw std::runtime_error("Failed to create temporary stdin file");
            }

            if (!stdin_data->empty()
                && std::fwrite(stdin_data->data(), 1, stdin_data->size(),
                               stdin_file.get()) != stdin_data->size()) {
                throw std::runtime_error("Failed to write temporary stdin file");
            }

            std::rewind(stdin_file.get());

            FILE* stdin_fp = stdin_file.get();
            target = bp::child(target_invocation, bp::args(args),
                               bp::std_in < stdin_fp,
                               bp::std_out > stdout, bp::std_err > stderr,
                               group);
        } else {
            target = bp::child(target_invocation, bp::args(args),
                               bp::std_in < bp::null,
                               bp::std_out > stdout, bp::std_err > stderr,
                               group);
        }

        result.started = true;

        if (!wait_for_wrapper(target, std::chrono::milliseconds(timeout_ms))) {
            result.timed_out = true;
            group.terminate();
            target.wait_for(std::chrono::milliseconds(100));
            std::cout << "TIME OUT: " << timeout_ms << std::endl;
            get_shared_memory().set_termination(target_termination::timeout);
        }

        const auto termination = get_shared_memory().get_termination();

        if (result.timed_out) {
            result.termination = target_termination::timeout;
        } else if (termination && *termination != target_termination::normal) {
            result.termination = *termination;
        } else if (target.exit_code() == 0) {
            get_shared_memory().set_termination(target_termination::normal);
            result.termination = target_termination::normal;
        } else {
            get_shared_memory().set_termination(target_termination::crash);
            result.termination = target_termination::crash;
        }

        result.header_valid =
            get_shared_memory().header()->verify_header_checksum();

        group.detach();
    } catch (const std::exception& e) {
        std::cout << "Failed to execute target: " << e.what() << std::endl;
        result.started = false;
        result.header_valid = false;
        result.termination = target_termination::crash;
    }

    result.wall_ms = static_cast<std::size_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - wall_start).count());

    return result;
}

}  // namespace connection
