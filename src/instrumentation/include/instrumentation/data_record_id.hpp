#pragma once

#include <utility/basic_numeric_types.hpp>

namespace instrumentation {

enum class data_record_id : natural_8_bit {
    invalid     = 0,
    termination = 1,
    condition   = 2,
    br_instr    = 3,
    stdin_bytes = 4
};

}
