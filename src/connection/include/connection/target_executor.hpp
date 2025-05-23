#pragma once

#include <connection/shared_memory.hpp>

namespace connection {

struct target_executor {
    target_executor(std::string target_invocation);

    void init_shared_memory(std::size_t size);
    void set_timeout(natural_16_bit const timeout_ms_);
    void execute_target();

    shared_memory& get_shared_memory()
    {
        return shm;
    }

    natural_16_bit timeout_ms;

   private:
    std::string target_invocation;
    shared_memory shm;
};

}  // namespace connection
