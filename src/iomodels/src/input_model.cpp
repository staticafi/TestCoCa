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
    byte_count_type num_bytes;
    src >> num_bytes;
    bytes.resize(num_bytes);
    src.deliver_bytes(bytes.data(), num_bytes);
}


input_model::byte_count_type input_model::max_bytes() const {
    return m_max_bytes;
}
}
