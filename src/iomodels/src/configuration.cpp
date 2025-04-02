#include <iomodels/configuration.hpp>
#include <target/target_termination.hpp>

#include "connection/shared_memory.hpp"
#include "utility/basic_numeric_types.hpp"

using namespace instrumentation;

namespace iomodels {
    bool configuration::operator==(configuration const&  other) const
    {
        return max_exec_milliseconds == other.max_exec_milliseconds &&
        max_exec_megabytes == other.max_exec_megabytes;
    }

    void configuration::invalidate_shared_memory_size_cache() const {
        shared_memory_size_cache.reset();
    }

    void configuration::save_target_config(connection::shared_memory& dest) const {
        dest << max_exec_megabytes;
    }

    void configuration::load_target_config(connection::shared_memory& src) {
        return;
        src >> max_exec_megabytes;
    }
}