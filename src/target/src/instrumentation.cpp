#include <cstdint>
#include <target/instrumentation_types.hpp>
#include <target/target.hpp>

using namespace instrumentation;

extern "C" {

void __testcoca_process_br_instr(uint32_t const id, bool const direction)
{
    target->process_br_instr(id, static_cast<condition_coverage>(direction));
}

void __testcoca_process_ver_error()
{
    target->process_ver_error();
}

void __testcoca_process_goal(uint32_t const id)
{
    target->process_goal(id);
}

}
