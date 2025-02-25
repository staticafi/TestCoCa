#include <cstdint>
#include <instrumentation/target.hpp>
#include <instrumentation/instrumentation_types.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/invariants.hpp>

using namespace instrumentation;

extern "C" {

void __sbt_fizzer_process_br_instr(uint32_t const id,
                                   bool const direction)
{
    sbt_fizzer_target->process_br_instr(id, direction);
}

}
