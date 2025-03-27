#pragma once

#include <utility/math.hpp>

#include "connection/shared_memory.hpp"
#include "target/instrumentation_types.hpp"

namespace  iomodels {

struct input_model
{
    using byte_count_type    = uint64_t;
    using type_of_input_bits = instrumentation::type_of_input_bits;
    using input_types_vector = std::vector<type_of_input_bits>;

    input_model(byte_count_type  max_bytes_);

    void  clear();
    void  save(connection::shared_memory&  dest) const;
    void  load(connection::shared_memory&  src);
    void  read(natural_8_bit*  ptr, instrumentation::type_of_input_bits  type, connection::shared_memory& dest);

    vecu8 const&  get_bytes() const { return bytes; }
    input_types_vector const&  get_types() const { return types; }
    byte_count_type  num_bytes_read() const { return cursor; }

    void  set_bytes(vecu8 const&  bytes_) { bytes = bytes_; }
    byte_count_type max_bytes() const;

private:
    byte_count_type m_max_bytes;
    byte_count_type  cursor;
    vecu8  bytes;
    input_types_vector  types;
};

using  input_model_ptr = std::unique_ptr<input_model>;

}
