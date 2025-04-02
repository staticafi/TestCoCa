#pragma once

#include "connection/shared_memory.hpp"
#include "utility/basic_numeric_types.hpp"


namespace iomodels {

struct  configuration
{
    natural_32_bit required_shared_memory_size() const;
    void invalidate_shared_memory_size_cache() const;

    void save_target_config(connection::shared_memory& dest) const;
    void load_target_config(connection::shared_memory& src);
    void save_client_config(connection::shared_memory& dest) const;

    bool operator==(configuration const&  other) const;
    bool operator!=(configuration const&  other) const { return !(*this == other);}

    mutable std::optional<natural_32_bit> shared_memory_size_cache;

    natural_16_bit max_exec_milliseconds { 250 };
    natural_16_bit  max_exec_megabytes { 1024 };
};

}
