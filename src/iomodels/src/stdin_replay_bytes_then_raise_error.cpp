#include <iomodels/stdin_replay_bytes_then_raise_error.hpp>
#include <iostream>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <target/target_termination.hpp>

using namespace connection;
using namespace instrumentation;

namespace  iomodels {


stdin_replay_bytes_then_raise_error::stdin_replay_bytes_then_raise_error(byte_count_type const  max_bytes_)
    : stdin_base{ max_bytes_ }
    , cursor(0U)
    , bytes()
    , types()
{}


void  stdin_replay_bytes_then_raise_error::clear()
{
    cursor = 0U;
    bytes.clear();
    types.clear();
}

void  stdin_replay_bytes_then_raise_error::save(shared_memory& dest) const
{
    INVARIANT(bytes.size() <= max_bytes());

    dest << (byte_count_type)bytes.size();
    dest.accept_bytes(bytes.data(),(byte_count_type)bytes.size());

    dest << (byte_count_type)types.size();
    dest.accept_bytes(types.data(), (byte_count_type)types.size());
}

void  stdin_replay_bytes_then_raise_error::load(shared_memory&  src)
{
    byte_count_type  num_bytes;
    src >> num_bytes;
    bytes.resize(num_bytes);
    src.deliver_bytes(bytes.data(), num_bytes);
}

std::size_t stdin_replay_bytes_then_raise_error::min_flattened_size() const {
    return sizeof(input_types_vector::value_type) + 1;
}

void  stdin_replay_bytes_then_raise_error::read(natural_8_bit*  ptr,
                                                type_of_input_bits const type,
                                                shared_memory& dest)
{
    natural_8_bit const count = num_bytes(type);

    if (cursor + count > max_bytes()) {
        dest.set_termination(target_termination::boundary_condition_violation);
        exit(0);
    }
    if (!dest.can_accept_bytes(count + 2)) {
        dest.set_termination(target_termination::medium_overflow);
        exit(0);
    }
    if (cursor + count > bytes.size()) {
        dest.set_termination(target_termination::insufficient_data);
        exit(0);
    }

    memcpy(ptr, bytes.data() + cursor, count);
    cursor += count;
}


}
