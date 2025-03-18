#include <target/target.hpp>
#include <iostream>

#include "iomodels/stdin_replay_bytes_then_raise_error.hpp"

using namespace iomodels;

namespace instrumentation {

target::target()
{
}

void target::process_br_instr(br_instr_id id, bool covered_branch)
{
    std::lock_guard lock(mutex);
    if (stdin_model->num_bytes_read() == 0) return;

    if (!shared_memory.can_accept_bytes(
            br_instr_coverage_info::flattened_size())) {
        shared_memory.set_termination(target_termination::medium_overflow);
        exit(0);
    }

    // TODO use hashmap
    shared_memory << id << covered_branch;
}

void target::process_ver_error()
{
    std::lock_guard lock(mutex);
    shared_memory.set_termination(target_termination::ver_error_reached);
    exit(0);
}

void target::on_read(natural_8_bit* ptr, type_of_input_bits const type)
{
    std::lock_guard lock(mutex);
    stdin_model->read(ptr, type, shared_memory);
}

void target::load_stdin()
{
    stdin_model->load(shared_memory);
}

void target::load_config() {
    config.load_target_config(shared_memory);
    stdin_model = std::make_unique<stdin_replay_bytes_then_raise_error>(100); //TODO dummy value
}

}  // namespace instrumentation
