#pragma once

#include <utility/basic_numeric_types.hpp>
#include <iosfwd>

namespace  instrumentation {

using  br_instr_id = uint32_t;
using  goal_id = uint32_t;

enum condition_coverage : uint8_t {
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

struct  goal_coverage_info
{
    goal_coverage_info(uint32_t id, uint8_t);

    static size_t flattened_size();

    br_instr_id  id;
    condition_coverage  coverage;
};

enum struct type_of_input_bits : uint8_t
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

    UINT128 = 9U,
    SINT128 = 10U,

    FLOAT32 = 11U,
    FLOAT64 = 12U,
};

uint8_t num_bytes(type_of_input_bits type);
}