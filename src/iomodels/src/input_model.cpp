#include <iomodels/input_model.hpp>
#include <iostream>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <target/target_termination.hpp>

using namespace connection;
using namespace instrumentation;

namespace  iomodels {


input_model::input_model(uint64_t const  max_bytes_)
    : m_max_bytes(max_bytes_)
    , cursor(0U) {}


void input_model::clear()
{
    cursor = 0U;
    bytes.clear();
}

void input_model::load(shared_memory&  src)
{
    uint64_t num_bytes;
    src >> num_bytes;
    src >> input_count;
    bytes.resize(num_bytes, 0);
    src.deliver_bytes(bytes.data(), num_bytes);
}


uint64_t input_model::max_bytes() const {
    return m_max_bytes;
}
}
