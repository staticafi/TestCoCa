#include <fstream>
#include <target/target.hpp>

#include "iomodels/input_model.hpp"

using namespace iomodels;

namespace instrumentation {

target::target()
{
}

void target::process_br_instr(const br_instr_id id, const condition_coverage covered_branch)
{
    std::lock_guard lock(mutex);

    if (!shared_memory.can_accept_bytes(
            br_instr_coverage_info::flattened_size())) {
        shared_memory.set_termination(target_termination::medium_overflow);
        exit(0);
    }

    //TODO insert
    auto [it, inserted] = index_map.insert({id, index});

    auto i = it->second;

    auto mem = (br_instr_coverage_info*)(shared_memory.get_memory() +
                sizeof(instrumentation::target_termination) * 2 +
                sizeof(natural_32_bit));

    if (inserted) {
        ++index;
        mem[i] = br_instr_coverage_info(id, covered_branch);
        return;
    }

    auto saved_cov = mem[i].coverage;

    if (saved_cov != instrumentation::BOTH &&
        saved_cov != covered_branch) {
        mem[i].coverage = instrumentation::BOTH;
    }
}

void target::process_ver_error()
{
    std::lock_guard lock(mutex);
    shared_memory.set_termination(target_termination::ver_error_reached);
    exit(0);
}

void target::load_stdin()
{
    stdin_model->load(shared_memory);
}

void target::load_config() {
    config.load_target_config(shared_memory);
    stdin_model = std::make_unique<input_model>(100); //TODO dummy value
}

}  // namespace instrumentation
