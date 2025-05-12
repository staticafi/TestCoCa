#include <target/instrumentation_types.hpp>
#include <utility/invariants.hpp>
#include <ostream>
#include <iomanip>
#include <iostream>

namespace  instrumentation {


br_instr_coverage_info::br_instr_coverage_info(const br_instr_id id, const condition_coverage coverage)
    : id(id)
    , coverage(coverage)
{}

size_t br_instr_coverage_info::flattened_size() {
    return sizeof(id) + sizeof(condition_coverage);
}

natural_8_bit num_bytes(type_of_input_bits type)
{
    switch (type)
    {
        case type_of_input_bits::BOOLEAN:
        case type_of_input_bits::UINT8:
        case type_of_input_bits::SINT8:
            return 1U;
        case type_of_input_bits::UINT16:
        case type_of_input_bits::SINT16:
            return 2U;
        case type_of_input_bits::UINT32:
        case type_of_input_bits::SINT32:
        case type_of_input_bits::FLOAT32:
            return 4U;
        case type_of_input_bits::UINT64:
        case type_of_input_bits::SINT64:
        case type_of_input_bits::FLOAT64:
            return 8U;
        case type_of_input_bits::UINT128:
        case type_of_input_bits::SINT128:
            return 16U;
        default: { UNREACHABLE(); return 0U; }
    }
}

}
