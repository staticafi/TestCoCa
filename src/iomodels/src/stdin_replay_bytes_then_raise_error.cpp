#include <iomodels/stdin_replay_bytes_then_raise_error.hpp>
#include <iostream>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <instrumentation/data_record_id.hpp>
#include <instrumentation/target_termination.hpp>

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

template <typename Medium>
void  stdin_replay_bytes_then_raise_error::save_(Medium& dest) const
{
    INVARIANT(bytes.size() <= max_bytes());

    dest << (byte_count_type)bytes.size();
    dest.accept_bytes(bytes.data(),(byte_count_type)bytes.size());

    dest << (byte_count_type)types.size();
    dest.accept_bytes(types.data(), (byte_count_type)types.size());
}

template void stdin_replay_bytes_then_raise_error::save_(shared_memory&) const;


void  stdin_replay_bytes_then_raise_error::save(shared_memory& dest) const
{
    save_(dest);
}

template <typename Medium>
void  stdin_replay_bytes_then_raise_error::load_(Medium&  src)
{
    byte_count_type  num_bytes;
    src >> num_bytes;
    std::cout << num_bytes << std::endl;
    bytes.resize(num_bytes);
    src.deliver_bytes(bytes.data(), num_bytes);

    ASSUMPTION(bytes.size() <= max_bytes());

    byte_count_type  num_types;
    src >> num_types;
    types.resize(num_types);
    src.deliver_bytes(types.data(), num_types);
}

template void stdin_replay_bytes_then_raise_error::load_(shared_memory&);

void  stdin_replay_bytes_then_raise_error::load(shared_memory&  src)
{
    load_(src);
}


template <typename Medium>
bool  stdin_replay_bytes_then_raise_error::load_record_(Medium& src) {
    if (!src.can_deliver_bytes(1))
        return false;
    natural_8_bit type_id;
    src >> type_id;
    type_of_input_bits const type = from_id(type_id);
    natural_8_bit const count = num_bytes(type);
    if (!src.can_deliver_bytes(count))
        return false;
    types.push_back(type);
    size_t old_size = bytes.size();
    bytes.resize(old_size + count);
    src.deliver_bytes(bytes.data() + old_size, count);
    return true;
}


template bool stdin_replay_bytes_then_raise_error::load_record_(shared_memory&);

bool  stdin_replay_bytes_then_raise_error::load_record(shared_memory&  src) {
    return load_record_(src);
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
        std::cout << "insufficient_data" << std::endl;
        dest.set_termination(target_termination::insufficient_data);
        exit(0);
    }

    std::cout << "read ok" << std::endl;
    memcpy(ptr, bytes.data() + cursor, count);
    dest << data_record_id::stdin_bytes << to_id(type);
    dest.accept_bytes(bytes.data() + cursor, count);
    cursor += count;
    types.push_back(type);
}


}
