#pragma once

#include <iomodels/stdin_base.hpp>
#include <utility/math.hpp>

namespace  iomodels {

struct stdin_replay_bytes_then_raise_error : public stdin_base
{
    stdin_replay_bytes_then_raise_error(byte_count_type  max_bytes_);

    void  clear() override;
    void  save(connection::shared_memory&  dest) const override;
    void  load(connection::shared_memory&  src) override;
    bool  load_record(connection::shared_memory&  src) override;
    std::size_t min_flattened_size() const override;
    void  read(natural_8_bit*  ptr, type_of_input_bits  type, connection::shared_memory& dest) override;

    vecu8 const&  get_bytes() const override { return bytes; }
    input_types_vector const&  get_types() const override { return types; }
    byte_count_type  num_bytes_read() const override { return cursor; }

    void  set_bytes(vecu8 const&  bytes_) override { bytes = bytes_; }

private:
    byte_count_type  cursor;
    vecu8  bytes;
    input_types_vector  types;
};


}
