#pragma once

#include <utility/basic_numeric_types.hpp>
#include <iosfwd>

namespace  instrumentation {

using  br_instr_id = natural_32_bit;

enum condition_coverage : natural_8_bit {
    FALSE,
    TRUE,
    BOTH
};

struct  br_instr_coverage_info
{
    br_instr_coverage_info(br_instr_id id, condition_coverage coverage);

    static size_t flattened_size();

    br_instr_id  id;
    condition_coverage  coverage;
};

enum struct type_of_input_bits : natural_8_bit
{
    BOOLEAN = 0U,

    UINT8 = 1U,
    SINT8 = 2U,

    UINT16 = 3U,
    SINT16 = 4U,

    UINT32 = 5U,
    SINT32 = 6U,

    UINT64 = 7U,
    SINT64 = 8U,

    FLOAT32 = 9U,
    FLOAT64 = 10U,

    // Unknown types:

    UNTYPED8 = 11U,
    UNTYPED16 = 12U,
    UNTYPED32 = 13U,
    UNTYPED64 = 14U
};

type_of_input_bits  from_id(natural_8_bit  id);
type_of_input_bits  from_string(std::string&&);
natural_8_bit  num_bytes(type_of_input_bits  type);
bool is_float(type_of_input_bits  type);
bool is_signed(type_of_input_bits  type);

}