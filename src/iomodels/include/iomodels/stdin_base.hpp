#pragma once

#include <connection/shared_memory.hpp>
#include <target/instrumentation_types.hpp>
#include <memory>
#include <utility/math.hpp>

namespace iomodels {

struct stdin_base {
    using byte_count_type    = natural_32_bit;
    using type_of_input_bits = instrumentation::type_of_input_bits;
    using input_types_vector = std::vector<type_of_input_bits>;

    explicit stdin_base(byte_count_type const max_bytes_)
        : m_max_bytes{max_bytes_}
    {
    }
    virtual ~stdin_base() = default;

    virtual void clear()                                     = 0;
    virtual void save(connection::shared_memory& dest) const = 0;
    virtual void load(connection::shared_memory& src)        = 0;
    virtual size_t min_flattened_size() const                = 0;
    virtual void read(natural_8_bit* ptr, type_of_input_bits type,
                      connection::shared_memory& dest)       = 0;

    virtual vecu8 const& get_bytes() const              = 0;
    virtual input_types_vector const& get_types() const = 0;
    virtual byte_count_type num_bytes_read() const      = 0;

    virtual void set_bytes(vecu8 const& bytes) = 0;

    byte_count_type max_bytes() const
    {
        return m_max_bytes;
    }

   private:
    byte_count_type m_max_bytes;
};

using stdin_base_ptr = std::unique_ptr<stdin_base>;

}  // namespace iomodels
