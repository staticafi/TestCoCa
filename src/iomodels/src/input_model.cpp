#include <iomodels/input_model.hpp>
#include <iostream>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <target/target_termination.hpp>

using namespace connection;
using namespace instrumentation;

namespace  iomodels {


input_model::input_model(byte_count_type const  max_bytes_)
    : m_max_bytes(max_bytes_)
    , cursor(0U) {}


void  input_model::clear()
{
    cursor = 0U;
    bytes.clear();
    types.clear();
}

void  input_model::save(shared_memory& dest) const
{
    INVARIANT(bytes.size() <= max_bytes());

    dest << (byte_count_type)bytes.size();
    dest.accept_bytes(bytes.data(),(byte_count_type)bytes.size());

    dest << (byte_count_type)types.size();
    dest.accept_bytes(types.data(), (byte_count_type)types.size());
}

void  input_model::load(shared_memory&  src)
{
    byte_count_type  num_bytes;
    src >> num_bytes;
    bytes.resize(num_bytes);
    src.deliver_bytes(bytes.data(), num_bytes);
}

std::size_t input_model::min_flattened_size() const {
    return sizeof(input_types_vector::value_type) + 1;
}

void  input_model::read(natural_8_bit*  ptr,
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

input_model::byte_count_type input_model::max_bytes() const {
    return m_max_bytes;
}
}
