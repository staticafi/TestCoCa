#include <cstdint>
#include <instrumentation/instrumentation_types.hpp>
#include <instrumentation/target.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/invariants.hpp>

using namespace instrumentation;

extern "C" {

void __qmi_process_br_instr(uint32_t const id, bool const direction)
{
    target->process_br_instr(id, direction);
}

void __qmi_process_ver_error(uint32_t const id)
{
    target->process_ver_error(id);
}

}
