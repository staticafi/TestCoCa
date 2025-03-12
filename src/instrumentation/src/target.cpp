#include <instrumentation/data_record_id.hpp>
#include <instrumentation/target.hpp>
#include <iomodels/models_map.hpp>
#include <iostream>
#include <utility/assumptions.hpp>
#include <utility/hash_combine.hpp>
#include <utility/invariants.hpp>

#include "iomodels/stdin_replay_bytes_then_raise_error.hpp"

using namespace iomodels;

namespace instrumentation {

target::target()
{
}

void target::process_br_instr(location_id id, bool covered_branch)
{
    if (stdin_model->num_bytes_read() == 0) return;

    if (!shared_memory.can_accept_bytes(
            br_instr_coverage_info::flattened_size())) {
        shared_memory.set_termination(target_termination::medium_overflow);
        exit(0);
    }

    shared_memory << data_record_id::br_instr << id << covered_branch;
}

void target::process_ver_error()
{
    shared_memory.set_termination(target_termination::ver_error_reached);
    exit(0);
}

void target::on_read(natural_8_bit* ptr, type_of_input_bits const type)
{
    // TODO remove sending shared_memory
    stdin_model->read(ptr, type, shared_memory);
}

void target::on_write(natural_8_bit const* ptr,
                           type_of_input_bits const type)
{
    stdout_model->write(ptr, type, shared_memory);
}

void target::load_stdin()
{
    assert(stdin_model);
    stdin_model->load(shared_memory);
}

void target::load_stdout() {
    stdout_model->load(shared_memory);
}

void target::load_config() {
    config.load_target_config(shared_memory);
    stdin_model = std::make_unique<stdin_replay_bytes_then_raise_error>(100); //TODO dummy value
    stdout_model = std::make_unique<stdout_void>();
}

}  // namespace instrumentation
