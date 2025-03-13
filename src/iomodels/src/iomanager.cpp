#include <iomodels/iomanager.hpp>
#include <iomodels/stdout_void.hpp>
#include <utility/hash_combine.hpp>
#include <utility/assumptions.hpp>
#include <utility/timeprof.hpp>

#include "iomodels/stdin_replay_bytes_then_raise_error.hpp"

using namespace connection;
using namespace instrumentation;

namespace  iomodels {


iomanager::iomanager()
    : config{}
    , stdin_ptr(nullptr)
    , stdout_ptr(nullptr)
{}


iomanager&  iomanager::instance()
{
    static iomanager  man;
    return man;
}


void  iomanager::set_config(configuration const&  cfg)
{
    config = cfg;
    config.invalidate_shared_memory_size_cache();

    stdin_ptr = nullptr;
    stdout_ptr = nullptr;
}

bool  iomanager::load_br_instr_trace_record(shared_memory& src) {
    if (!src.can_deliver_bytes(br_instr_coverage_info::flattened_size()))
        return false;
    br_instr_coverage_info  info { invalid_location_id() };
    natural_8_bit uchr;
    src >> info.br_instr_id;
    src >> uchr; info.covered_branch = uchr != 0U;
    return true;
}

void  iomanager::load_results(shared_memory& src) {
    TMPROF_BLOCK();
    bool  invalid_record_reached{ false };
    //TODO remove switch - leave data record id for error detection
    while (!invalid_record_reached && !src.exhausted()) {
        invalid_record_reached = !load_br_instr_trace_record(src);
    }
}

stdin_base*  iomanager::get_stdin() const
{
    if (stdin_ptr == nullptr)
        stdin_ptr = std::make_unique<stdin_replay_bytes_then_raise_error>(100);
    return stdin_ptr.get();
}

stdout_base*  iomanager::get_stdout() const
{
    if (stdout_ptr == nullptr)
        stdout_ptr = std::make_unique<stdout_void>();
    return stdout_ptr.get();
}



}
