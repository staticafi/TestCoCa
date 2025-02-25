#include <instrumentation/data_record_id.hpp>
#include <instrumentation/target.hpp>
#include <iomodels/models_map.hpp>
#include <utility/assumptions.hpp>
#include <utility/hash_combine.hpp>
#include <utility/invariants.hpp>

using namespace iomodels;

namespace instrumentation {

fuzz_target::fuzz_target()
{
}

void fuzz_target::process_br_instr(location_id id, bool covered_branch)
{
    if (stdin_model->num_bytes_read() == 0) return;

    if (!shared_memory.can_accept_bytes(
            br_instr_coverage_info::flattened_size())) {
        shared_memory.set_termination(target_termination::medium_overflow);
        exit(0);
    }

    shared_memory << data_record_id::br_instr << id << covered_branch;
}

void fuzz_target::on_read(natural_8_bit* ptr, type_of_input_bits const type)
{
    stdin_model->read(ptr, type, shared_memory);
}

void fuzz_target::on_write(natural_8_bit const* ptr,
                           type_of_input_bits const type)
{
    stdout_model->write(ptr, type, shared_memory);
}

void fuzz_target::load_stdin()
{
    stdin_model->load(shared_memory);
}

}  // namespace instrumentation
