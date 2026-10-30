#pragma once

#include <connection/shared_memory.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace connection {

struct execution_result
{
    instrumentation::target_termination termination =
        instrumentation::target_termination::normal;
    bool timed_out = false;
    bool started = false;
    bool header_valid = false;
    std::size_t wall_ms = 0;
};

struct target_executor {
    target_executor(std::string target_invocation);

    void init_shared_memory(std::size_t size);
    void set_timeout(std::size_t timeout_ms_);
    execution_result execute_target(
        const std::vector<std::string>& args,
        const std::optional<std::string>& stdin_data = std::nullopt);

    shared_memory& get_shared_memory()
    {
        return shm;
    }

    std::uint64_t timeout_ms;

   private:
    std::string target_invocation;
    shared_memory shm;
};

}  // namespace connection
